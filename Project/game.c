//game.c

#include "mipslab.h"

void run() {
    int best_scores[3] = {0, 0, 0};

    while (1) {
        clear_text_buffer();
        int choice = display_menu();
        if (choice == 0) {
            clear_display();
            display_update();
            // PRESS BTN4 TO EXIT MENU
            while (1) {
                clear_text_buffer();

                display_highscore(&best_scores);
                if (PORTD &= 0x20) {
                    choice = 2;
                    break;
                }

            }
            if (choice == 1) {


                clear_text_buffer();
                score = 0;
                jump_flag = 0;
                dash_flag = 0;
                crash_flag = 0;
                bob_x = 0;
                bob_y = BOB_MIN_Y;
                TIME_BETWEEN_OBSTACLES = 10;
                display_string(3, "Game!");
                display_update();
                quicksleep(10000000);
                clear_text_buffer();

                T2CONSET = 0x8000; // Starts timer

                obstacles[0] = ROCK;
                obstacle_pos[0] = 115;

                int obnum;
                for (obnum = 1; obnum < 12; obnum++) {
                    obstacles[obnum] = NOTHING;
                    obstacle_pos[obnum] = -10;
                }

                copy_board(board, board_copy);
                place_obstacle(obstacle_pos[0], obstacles[0]);
                add_bob_to_board(bob_x, bob_y);
                display_current_board();
                display_image(0, board_to_display);
                quicksleep(10000);

                while (!crash_flag) {

                    copy_board(board, board_copy);

                    int i;
                    for (i = 0; i < LENGTH_OF_OBS_ARR; i++) {
                        int obstacle_type = obstacles[i];
                        int obstacle_position = obstacle_pos[i];
                        if (obstacle_pos[i] >= -10 && obstacles[i] != NOTHING) {
                            place_obstacle(obstacle_position, obstacle_type);
                            obstacle_pos[i]--;
                            if (obstacle_position < -7) {
                                obstacle_pos[i] = -10;
                                obstacles[i] = NOTHING;
                            }
                        }
                    }

                    // When BTN4 is pressed...
                    if (PORTD &= 0x80) {
                        jump_flag = 1;
                    }
                    // When BTN3 is pressed...
                    if (PORTD &= 0x40) {
                        dash_flag = 1;
                    }

                    move(&dash_flag, &jump_flag);

                    crash_flag = add_bob_to_board(bob_x, bob_y);       // Check if game over
                    display_current_board();
                    add_score(128 - 40, 0);
                    display_image(0, board_to_display);

                }
                // HIGHSCORE
                update_highscore(&best_scores, score);
                display_highscore(&best_scores);

                int restart = 0;
                while (!restart) {
                    restart = PORTD &= 0x20;
                }
            }
        }
    }
