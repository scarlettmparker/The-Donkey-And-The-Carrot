#include <iostream>
#include <array>
#include <chrono>
#include <random>
#include <bitset>

//             ┌        ┐        ┘        └         -           |
enum { player, curve_0, curve_1, curve_2, curve_3, straight_0, straight_1 };
std::bitset<81> board[7];
std::bitset<81> occupancies;
std::mt19937 gen;

// board display
enum {
    a9, b9, c9, d9, e9, f9, g9, h9, i9,
    a8, b8, c8, d8, e8, f8, g8, h8, i8,
    a7, b7, c7, d7, e7, f7, g7, h7, i7,
    a6, b6, c6, d6, e6, f6, g6, h6, i6,
    a5, b5, c5, d5, e5, f5, g5, h5, i5,
    a4, b4, c4, d4, e4, f4, g4, h4, i4,
    a3, b3, c3, d3, e3, f3, g3, h3, i3,
    a2, b2, c2, d2, e2, f2, g2, h2, i2,
    a1, b1, c1, d1, e1, f1, g1, h1, i1,
};

// helper coordinate stuff
const char *square_to_coordinates[] = {
    "a9", "b9", "c9", "d9", "e9", "f9", "g9", "h9", "i9",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8", "i8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", "i7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", "i6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", "i5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", "i4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", "i3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", "i2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "i1"
};

constexpr std::array<const char *, 6> dice_move = {
    "CARROT", "CARROT", "CARROT", "CURVE", "CURVE", "STRAIGHT"
};

constexpr std::array<const char *, 7> piece = {
    "PLAYER", "CURVE 0", "CURVE 1", "CURVE 2", "CURVE 3", "STRAIGHT 0", "STRAIGHT 1"
};

constexpr std::array<const char *, 7> piece_char = {
    "P", "0", "1", "2", "3", "4", "5"
};

typedef struct {
    int moves[16];
    int count;
} moves;

/**
 * Function to print an individual bitset.
 *
 * @param bitset Bitset to print off.
 */
void print_bitset(std::bitset<81> bitset) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int square = i * 9 + j;
            printf(" %d", bitset[square] ? 1 : 0);
        }
        printf("\n");
    }
}

/**
 * Function to print all the bitsets.
 * Loops over all bitsets and prints the game state.
 */
void print_bitsets() {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int square = i * 9 + j;
            const char *display_char = ".";

            for (int b = 0; b < 7; b++) {
                if (board[b][square]) {
                    display_char = piece_char[b];
                    break;
                }
            }

            printf(" %s", display_char);
        }
        printf("\n");
    }
}

/**
 * Get the bitset index (piece type) by square.
 *
 * @param square square to check piece type on.
 * @return -1 for no piece, 0-6 for piece types.
 */
int get_bitset_index(int square) {
    if (square < 0 || square >= 81) {
        return -2;
    }

    for (int i = 0; i < 7; ++i) {
        if (board[i].test(square)) {
            return i;
        }
    }

    return -1;
}

/**
 * Get the file of a square.
 *
 * @param square The square to check.
 * @returns Numeric value (0 to 8) of the file.
 */
int get_file(int square) {
    if (square < 0 || square >= 81) return -1;
    return square % 9;
}

/**
 * Get the rank of a square.
 *
 * @param square The square to check.
 * @returns Numeric value (0 to 8) of the rank.
 */
int get_rank(int square) {
    if (square < 0 || square >= 81) return -1;
    return 8 - (square / 9);
}

/**
 * Check if two squares are touching each other.
 *
 * @param target square to add to the board
 * @param source square currently on the board.
 * @return 1 if true, 0 if false. 
 */
int square_touching(int target, int source) {
    if (target == source) return 1;

    int diff = abs(target - source);
    if (diff == 1 || diff == 8 || diff == 9 || diff == 10) return 1;

    return 0;
}

/**
 * Adds a move to the current move list.
 *
 * @param move_list Struct containing moves and move count.
 * @param square Square to add to the move list.
 */
static inline void add_move(moves *move_list, int square) {
    move_list->moves[move_list->count] = square;
    move_list->count++;
}

/**
 * Generate all valid moves depending on the source square.
 *
 * @param source source square (previous move).
 * @param piece piece type (curves, lines).
 * @param move_list move list to modify.
 * @return 
 */
static inline void generate_valid_squares(int source, int piece, moves *move_list) {
    move_list->count = 0;

    int prev_move = get_bitset_index(source);

    if (prev_move == -1) {
        return;
    }

    int rank = get_rank(source);
    int file = get_file(source);

    // check directions for free squares
    int square_up = source - 9;
    int square_down = source + 9;
    int square_left = source - 1;
    int square_right = source + 1;

    // up right curve
    if (piece == curve_0) {
        if ((get_bitset_index(square_up + 1) == -1) && (prev_move == curve_2 || prev_move == curve_3 || prev_move == straight_1)) {;
            if (abs(get_rank(square_up + 1) - rank) == 1) {
                add_move(move_list, square_up);
            }
        }
        if ((get_bitset_index(square_left + 9) == -1) && (prev_move == curve_1 || prev_move == curve_2 || prev_move == straight_0)) {
            if (abs(get_file(square_left + 9) - file) == 1) {
                add_move(move_list, square_left);
            }
        }
    }

    // right down curve
    if (piece == curve_1) {
        if ((get_bitset_index(square_right + 9) == -1) && (prev_move == curve_0 || prev_move == curve_3 || prev_move == straight_0)) {
            if (abs(get_rank(square_right + 9) - rank) == 1) {
                add_move(move_list, square_right);
            }
        }
        if ((get_bitset_index(square_up - 1) == -1) && (prev_move == curve_2 || prev_move == curve_3 || prev_move == straight_1)) {
            if (abs(get_rank(square_up - 1) - rank) == 1) {
                add_move(move_list, square_up);
            }
        }
    }

    // right up curve
    if (piece == curve_2) {
        if ((get_bitset_index(square_right - 9) == -1) && (prev_move == curve_0 || prev_move == curve_3 || prev_move == straight_0)) {
            if (abs(get_rank(square_right - 9) - rank)) {
                add_move(move_list, square_right);
            }
        }
        if ((get_bitset_index(square_down - 1) == -1) && (prev_move == curve_0 || prev_move == curve_1 || prev_move == straight_1)) {
            if (abs(get_rank(square_down - 1) - rank) == 1) {
                add_move(move_list, square_down);
            }
        }
    }

    // down right curve
    if (piece == curve_3) {
        if ((get_bitset_index(square_left - 9) == -1) && (prev_move == curve_1 || prev_move == curve_2 || prev_move == straight_0)) {
            if (abs(get_file(square_left - 9) - file) == 1) {
                add_move(move_list, square_left);
            }
        }
        if ((get_bitset_index(square_down + 1) == -1) && (prev_move == curve_0 || prev_move == curve_1 || prev_move == straight_1)) {
            if (abs(get_file(square_down + 1) - file) == 1) {
                add_move(move_list, square_down);
            }
        }
    }

    // horizontal line
    if (piece == straight_0) {
        if ((get_bitset_index(square_right + 1) == -1) && (prev_move == curve_0 || prev_move == curve_3 || prev_move == straight_0)) {
            if (abs(get_file(square_right) - file) == 1 && get_file(square_right) != 8) {
                add_move(move_list, square_right);
            }
        }
        if ((get_bitset_index(square_left - 1) == -1) && (prev_move == curve_1 || prev_move == curve_2 || prev_move == straight_0)) {
            if (abs(get_file(square_left) - file) == 1 && get_file(square_left) != 0) {
                add_move(move_list, square_left);
            }
        }
    }

    // vertical line
    if (piece == straight_1) {
        if ((get_bitset_index(square_up - 9) == -1) && (prev_move == curve_2 || prev_move == curve_3 || prev_move == straight_1)) {
            if (abs(get_rank(square_up) - rank) == 1 && get_rank(square_up) != 8) {
                add_move(move_list, square_up);
            }
        }
        if ((get_bitset_index(square_down + 9) == -1) && (prev_move == curve_0 || prev_move == curve_1 || prev_move == straight_1)) {
            if (abs(get_rank(square_down) - rank) == 1 && get_rank(square_down) != 0) {
                add_move(move_list, square_down);
            }
        }
    }
}

/**
 * Makes a move given a dice roll.
 *
 * @param prev_move Previous move to check against for legal move generation.
 * @param num Dice roll number.
 * @returns Move made (to check for the next move).
 */
int make_move(int prev_move, int num) {
    std::uniform_int_distribution<> curve(1, 4);
    std::uniform_int_distribution<> straight(5, 6);

    int piece;
    moves move_list[1];

    // curve
    if (num == 3 || num == 4) {
        piece = curve(gen);
    }

    // straight_line
    if (num == 5) {
        piece = straight(gen);
    }

    generate_valid_squares(prev_move, piece, move_list);

    if (move_list->count == 0) {
        return prev_move;
    }

    for (int move_count = 0; move_count < move_list->count; move_count++) {
        int move = move_list->moves[move_count];

        // ensure pieces don't overwrite themselves.
        if (!(occupancies[move])) {
            board[piece].set(move, true);
            occupancies.set(move, true);
            prev_move = move;
            break;
        }
    }

    return prev_move;
}

int main() {
    // random number generator and seeding
    std::random_device rd;
    auto seed = rd() ^ std::chrono::high_resolution_clock::now().time_since_epoch().count();
    gen.seed(seed);

    std::array<int, 2> start_curve_values = {1, 3};

    std::uniform_int_distribution<> dice(3, 5);
    std::uniform_int_distribution<> curve(1, 4);
    std::uniform_int_distribution<> straight(5, 6);
    std::uniform_int_distribution<> start_curve(0, 1);

    int num = dice(gen);

    int piece;
    int move_count = 0;

    // curve
    if (num == 3 || num == 4) {
        piece = start_curve_values[start_curve(gen)];
    }

    // straight_line
    if (num == 5) {
        piece = straight(gen);
    }

    if (move_count == 0) {
        board[piece].set(a1, true);
        occupancies.set(a1, true);
    }

    int prev_move = a1;

    prev_move = make_move(prev_move, num);

    for (int i = 0; i < 10000; i++) {
        num = dice(gen);
        prev_move = make_move(prev_move, num);
    }
    
    print_bitsets();
    return 0;
}