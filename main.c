#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "map.h"
#include "movelist.h"
#include "util.h"

#define FRAME_RATE 30

// Set the player move speed in steps per second
#define MOVE_SPEED 10

// functions for our queue
void add(queue_t *q, coord_t c){
  node_c *new = malloc(sizeof(node_c));
  new->c = c;
  new->next = NULL;
  if (q->head == NULL){
    q->head = new;
  } else {
  node_c *i;
  for (i = q->head; i->next != NULL; i = i->next){}
    i->next = new;
  }
}

coord_t take(queue_t *q){
  if (q->head != NULL){
  coord_t value = q->head->c;
  node_c *temp = q->head->next;
  free(q->head);
  q->head = temp;
  return value;
  }
}

void destroy(queue_t *q) {
  while (q->head != NULL) {
    node_c *temp = q->head;
    q->head = q->head->next;
    free(temp);
  }
}

void init(queue_t *q){
  q->head = NULL;
}

coord_t increment(coord_t c, dir_t d){
  coord_t new_c;
  if (d == DIR_N){
    new_c.x = c.x;
    new_c.y = c.y-1;
  }
  if (d == DIR_S){
    new_c.x = c.x;
    new_c.y = c.y+1;
  }
  if (d == DIR_E){
    new_c.x = c.x+1;
    new_c.y = c.y;
  }
  if (d == DIR_W){
    new_c.x = c.x-1;
    new_c.y = c.y;
  }
  return new_c;
} 

dir_t reverse_dir(dir_t d){
  switch (d) {
    case DIR_N:
      return DIR_S;
      break;
    case DIR_S:
      return DIR_N;
      break;
    case DIR_W:
      return DIR_E;
      break;
    case DIR_E:
      return DIR_W;
      break;
    default:
      return EMPTY;
      break;
  }
}

// Find a path from the player position to dest and add the necessary moves to the movelist
void plan_route(map_t map, coord_t player, coord_t dest, movelist_t* moves) {

  // Setting up array to store distances and visited statues
  int visited[map.height][map.width];
  for (int i = 0; i < map.height; i++){
    for (int j = 0; j < map.width; j++){
      visited[i][j] = 2000;
    }
  }

  // Initiate Breadth First Search
  queue_t *q = malloc(sizeof(queue_t));
  init(q);
  visited[player.y][player.x] = 0; 
  add(q, player);
  coord_t current;
  while (q->head != NULL){
    current = take(q);
    if (current.x == dest.x && current.y == dest.y){
      break;
    } else {
      coord_t dirs[] = {increment(current, DIR_N), increment(current, DIR_S), increment(current, DIR_W), increment(current, DIR_E)};
      for (int i = 0; i<4; i++){
        coord_t next = dirs[i];
        if (can_move(map, next) && visited[next.y][next.x] == 2000){
          visited[next.y][next.x] = visited[current.y][current.x] + 1;
          add(q, next);
        }
      }
    }
}
  destroy(q);
  // Begin Back Tracking
  coord_t position = dest;
  dir_t dir_back[] = {DIR_N, DIR_S, DIR_W, DIR_E};
  while (visited[position.y][position.x] != 0){
    coord_t next_back[] = {increment(position, DIR_N), increment(position, DIR_S), increment(position, DIR_W), increment(position, DIR_E)};
    for (int i=0; i<4; i++){
      if (visited[next_back[i].y][next_back[i].x] == visited[position.y][position.x]-1){
        position = next_back[i];
        movelist_add_front(moves, reverse_dir(dir_back[i]));
        break;
      }
    }
  }
}


int main(int argc, char** argv) {
  // Check for an argument
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <map file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Load a map
  map_t map;
  if (!load_map(argv[1], &map)) {
    fprintf(stderr, "Failed to load map %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  // Set up ncurses
  initscr();              // Initialize the standard window
  noecho();               // Don't display keys when they are typed
  nodelay(stdscr, true);  // Set up non-blocking input with getch()
  keypad(stdscr, true);   // Allow arrow keys
  curs_set(false);        // Hide the cursor

  // Accept mouse clicks
  mousemask(BUTTON1_CLICKED, NULL);

  // Set up colors for each cell type in the mapp
  start_color();
  init_pair(CELL_BLANK, COLOR_WHITE, COLOR_BLACK);
  init_pair(CELL_GRASS, COLOR_BLACK, COLOR_GREEN);
  init_pair(CELL_WALL, COLOR_BLACK, COLOR_WHITE);
  init_pair(CELL_LAKE, COLOR_BLACK, COLOR_BLUE);

  // Keep track of the player position
  coord_t player = map.start;

  // Keep track of where the current view is
  coord_t viewport = {0, 0};

  // Keep a list of moves
  movelist_t moves;
  movelist_init(&moves);

  // Keep track of how many frames the player must wait before moving again
  size_t frames_until_move = 0;

  // Start the game loop
  bool running = true;
  while (running) {
    // Check for user input
    int input;
    while ((input = getch()) != ERR) {
      if (input == 'q') {
        // Exit when the user types q
        running = false;
      
      } else if (input == KEY_UP) {
        // Clear the move queue and add one move north
        movelist_clear(&moves);
        movelist_add_front(&moves, DIR_N);
      
      } else if (input == KEY_DOWN) {
        // Clear the move queue and add one move south
        movelist_clear(&moves);
        movelist_add_front(&moves, DIR_S);
      
      } else if (input == KEY_LEFT) {
        // Clear the move queue and add one move west
        movelist_clear(&moves);
        movelist_add_front(&moves, DIR_W);

      } else if (input == KEY_RIGHT) {
        // Clear the move queue and add one move east
        movelist_clear(&moves);
        movelist_add_front(&moves, DIR_E);

      } else if (input == KEY_MOUSE) {
        // A mouse click was detected. Get the coordinates
        MEVENT m_event;
        if (getmouse(&m_event) == OK) {
          // Clear the movelist
          movelist_clear(&moves);

          // Calculate the destination the player should move to
          coord_t dest;
          dest.x = m_event.x / 2 + viewport.x;
          dest.y = m_event.y + viewport.y;

          // Plan a route to the destination
          plan_route(map, player, dest, &moves);
        }
      }
    }

    // Is it time to move?
    if (frames_until_move == 0) {
      // Yes. Grab a move if we have one
      dir_t move = movelist_take_front(&moves);
      if (move != EMPTY) {
        // Calculate the next player position
        coord_t next = player;
        if (move == DIR_N) {
          next.y--;
        } else if (move == DIR_E) {
          next.x++;
        } else if (move == DIR_S) {
          next.y++;
        } else if (move == DIR_W) {
          next.x--;
        }

        // Can the player move to the new position?
        if (can_move(map, next)) {
          // Yes. Update the player position.
          player = next;

          // Set a delay before the next move
          frames_until_move = FRAME_RATE / MOVE_SPEED;
        
        } else {
          // Clear the whole movelist when there's a collision
          movelist_clear(&moves);
        }
      }

    } else {
      // No. Decrement the frame count
      frames_until_move--;
    }

    // Display the map and player
    display_map(map, player, &viewport);

    // Display everything
    refresh();

    // Pause to limit frame rate
    sleep_ms(1000 / FRAME_RATE);
  }

  // Shut down
  endwin();

  // Clean up the movelist
  movelist_destroy(&moves);

  // Clean up the map
  free_map(&map);

}
