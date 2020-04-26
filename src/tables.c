#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attacks.h"
#include "bitboard.h"
#include "position.h"
#include "tables.h"
#include "types.h"

static u64 attack_rays_data[64][8];
static u64 in_between_LUT_data[64][64];

const u64 (*const attack_rays_ptr)[64][8] = &attack_rays_data;
const u64 (*const in_between_LUT_ptr)[64][64] = &in_between_LUT_data;

struct Attack_set attack_set;

void init_LUTs() {
    fill_attack_rays();
    fill_attack_sets();
    fill_inbetween_LUT();
}

void fill_attack_rays() {
    //Start with north and south rays, since we don't have to worry about wraps in this case
    u64 north_ray = FileABB ^ set_bit(a1);
    u64 south_ray = FileHBB ^ set_bit(h8);

    for (int sq = 0; sq < 64; ++sq) {
        attack_rays_data[sq][NORTH] = north_ray;
        north_ray <<= 1;
    }

    for (int sq = 63; sq >= 0; --sq) {
        attack_rays_data[sq][SOUTH] = south_ray;
        south_ray >>= 1;
    }

    //Northeast & east & southeast now, taking care because of wraps
    u64 noea_ray = Main_diagBB ^ set_bit(a1);
    u64 soea_ray = Anti_diagBB ^ set_bit(a8);
    u64 ea_ray = Rank1BB ^ set_bit(a1);

    for (int file = 0; file < 8; ++file) {
        u64 tmp_ne_ray = noea_ray;
        u64 tmp_se_ray = soea_ray;
        u64 tmp_e_ray = ea_ray;
        
        //NE and E
        for (int rank8 = a1; rank8 < 64; rank8 += 8) {
            attack_rays_data[rank8 + file][NORTHEAST] = tmp_ne_ray;
            tmp_ne_ray = shift_N(tmp_ne_ray);

            attack_rays_data[rank8 + file][EAST] = tmp_e_ray;
            tmp_e_ray = shift_N(tmp_e_ray);
        }

        //SE
        for (int rank8 = a8; rank8 >= 0; rank8 -= 8) {
            attack_rays_data[rank8 + file][SOUTHEAST] = tmp_se_ray;
            tmp_se_ray = shift_S(tmp_se_ray);
        }
        
        noea_ray = shift_E(noea_ray);
        soea_ray = shift_E(soea_ray);
        ea_ray = shift_E(ea_ray);
    }

    u64 nowe_ray = Anti_diagBB ^ set_bit(h1);
    u64 sowe_ray = Main_diagBB ^ set_bit(h8);
    u64 we_ray = Rank1BB ^ set_bit(h1);

    //Northwest & west &  Southwest
    for (int file = 7; file >= 0; --file) {
        u64 tmp_nw_ray = nowe_ray;
        u64 tmp_sw_ray = sowe_ray;
        u64 tmp_w_ray = we_ray;

        //NW and W
        for (int rank8 = a1; rank8 < 64; rank8 += 8) {
            attack_rays_data[rank8 + file][NORTHWEST] = tmp_nw_ray;
            tmp_nw_ray = shift_N(tmp_nw_ray);

            attack_rays_data[rank8 + file][WEST] = tmp_w_ray;
            tmp_w_ray = shift_N(tmp_w_ray);
        }

        //SW
        for (int rank8 = a8; rank8 >= 0; rank8 -= 8) {
            attack_rays_data[rank8 + file][SOUTHWEST] = tmp_sw_ray;
            tmp_sw_ray = shift_S(tmp_sw_ray);
        }

        nowe_ray = shift_W(nowe_ray);
        sowe_ray = shift_W(sowe_ray);
        we_ray = shift_W(we_ray);
    }
}


void fill_attack_sets() {
    for (int sq = 0; sq < 64; ++sq) {
        attack_set.pawn[WHITE][sq] = shift_NE(set_bit(sq)) | shift_NW(set_bit(sq));
        attack_set.pawn[BLACK][sq] = shift_SE(set_bit(sq)) | shift_SW(set_bit(sq));

        attack_set.knight[sq] = knight_attacks(sq);
        attack_set.king[sq] = king_attacks(sq);
    }
}

void fill_inbetween_LUT() {
    //Make sure the LUT is initialized to 0.
    memset(in_between_LUT_data, 0, sizeof(in_between_LUT_data[0][0]) * 64 * 64);

    //This LUT will be symmetric, in the sense that in_between_LUT[i][j] == in_between_LUT[j][i]
    //Furthermore, whenever i == j or abs(i - j) == 1, the value should be zero, so we don't need to set
    //those values. (since there are no bits in between two adjacent squares)
    for (int i = 0; i < 64; ++i) {
        for (int j = i + 2; j < 64; ++j) {
            enum Direction pos_direction;
            enum Direction neg_direction;

            //The inbetween square will only be set for squares on the same rank, file or diagonal,
            //since these are the only ones of relevance because that's how sliding pieces move in chess.

            //Squares on the same file, meaning the squares are in the north / south direction from each other.
            //The squares also have to have at least one square between them.
            if (sq_file(i) == sq_file(j) && abs(sq_rank(i) - sq_rank(j)) > 1) {
                pos_direction = NORTH;
                neg_direction = SOUTH;
            }

            //Squares on the same rank, meaning the squares are in east / west direction from each other.
            else if (sq_rank(i) == sq_rank(j) && abs(sq_file(i) - sq_file(j)) > 1) {
                pos_direction = EAST;
                neg_direction = WEST;
            }

            //Squares on the same (anti) diagonal.
            else if ((sq_rank(i) - sq_rank(j) == sq_file(j) - sq_file(i)) && abs(sq_rank(i) - sq_rank(j)) > 1) {
                pos_direction = NORTHWEST;
                neg_direction = SOUTHEAST;
            }

            //Squares on the same diagonalsq_file(i) - sq_file(j)
            else if ((sq_rank(i) - sq_rank(j) == sq_file(i) - sq_file(j)) && abs(sq_rank(i) - sq_rank(j)) > 1) {
                pos_direction = NORTHEAST;
                neg_direction = SOUTHWEST;
            }

            //The inbetween square will only be set for squares on the same rank, file or diagonal,
            //since these are the only ones of relevance because that's how sliding pieces move in chess.
            else {
                continue;
            }

            //Get the bits for the limits
            u64 lower_bit = (i < j) ? set_bit(i) : set_bit(j);
            u64 upper_bit = (i < j) ? set_bit(j) : set_bit(i);

            //Get the limits for the in_between bits
            u64 lower_lim = shift_bb(lower_bit, pos_direction);
            u64 upper_lim = shift_bb(upper_bit, neg_direction);

            //Fill the in between bits one at a time by shifting.
            u64 res = lower_lim;
            while (!(res & upper_lim)) {
                lower_lim = shift_bb(lower_lim, pos_direction);
                res |= lower_lim;
            }

            //The LUT tables are symmetric because the squares between square i and j are the same as the squares
            //between square j and i.
            in_between_LUT_data[i][j] = res;
            in_between_LUT_data[j][i] = res;
        }
    }
}