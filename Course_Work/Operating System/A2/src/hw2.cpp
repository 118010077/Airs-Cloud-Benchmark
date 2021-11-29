#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>
#include <termios.h>
#include <fcntl.h>

#define ROW 10
#define COLUMN 50
#define LOGNUM 9 //Number of logs
#define LOGLEN 15 //Length of logs.

pthread_mutex_t mutex;
pthread_mutex_t mutex_frog;
pthread_cond_t cond_fail;
bool endgaming = false;
struct Node{
    //x and y are the "coordinates".
    int x , y;
    Node( int _x , int _y ) : x( _x ) , y( _y ) {};
    Node(){} ;
} frog ;
// This structure helps to pass the arguments to the thread.
struct arg_struct{
    char dir;
    int order;
    int start;
    arg_struct(char, int, int);
    arg_struct();
};

// Initializer of the arg_struct structure.
arg_struct::arg_struct(char dir, int order, int start){
    this->dir = dir;
    this->order = order;
    this->start = start;
}

char map[ROW+10][COLUMN] ;
int startarray[LOGNUM];
void printMap(char (* map)[COLUMN]);
// Determine a keyboard is hit or not. If yes, return 1. If not, return 0.
// 'If condition' flow is useful.
int kbhit(void){
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}


void *frog_move(void *t){
    while (!endgaming){
        // Update the coordinate of the frog.
        int last_x = frog.x;
        int last_y = frog.y;
        if(kbhit()){
            char dir = getchar();
            pthread_mutex_lock(&mutex_frog);
            // Check whether the user want to exit the game.
            if(dir == 'q' || dir == 'Q'){
                endgaming = true;
                puts("Quit the Game!");
            } else if(dir == 'w' || dir == 'W'){
                frog.x -= 1;
            } else if(dir == 'a' || dir == 'A'){
                frog.y -= 1;
            } else if(dir =='s' || dir == 'S'){
                // If the frog is at the bottom, do nothing.
                if(frog.x >= ROW){
                    continue;
                }
                frog.x += 1;
            } else if(dir == 'd' || dir == 'D'){
                frog.y += 1;
            }

            pthread_mutex_unlock(&mutex_frog);
            // Update the value of the map
            pthread_mutex_lock(&mutex);
            map[last_x][last_y] = ' ';
            // Only continue after the status check.
            if(!endgaming) pthread_cond_wait(&cond_fail, &mutex);
            map[frog.x][frog.y] = '0';
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(NULL);

}

void *check_status(void *t){
    while (!endgaming){
        pthread_mutex_lock(&mutex);
        pthread_mutex_lock(&mutex_frog);

        // Situation 1: Frog at the logs or frog at the starting line.
        if(map[frog.x][frog.y] == '=' || frog.x == ROW)
            pthread_cond_signal(&cond_fail);
        // Situation 2: Frog reaches the opposite and win.
        else if(frog.x == 0){
            endgaming = true;
            map[frog.x][frog.y] = '0';
            puts("\033[2J\033[H");
            printMap(map);
            pthread_cond_signal(&cond_fail);
            puts("You win the game!");
        }
        // Situation 3: Frog drops into the river (space in the map).
        else if(map[frog.x][frog.y] == ' ' && frog.x != ROW){
            endgaming = true;
            map[frog.x][frog.y] = '!';
            puts("\033[2J\033[H");
            printMap(map);
            pthread_cond_signal(&cond_fail);
            puts("You lose the game!");
        }
        // Situation 4: Frog reaches the leftmost side or the rightmost side, and fails
        else if((frog.y < 0) || (frog.y >= COLUMN)){
            endgaming = true;
            puts("You lose the game!");
        }
        pthread_mutex_unlock(&mutex);
        pthread_mutex_unlock(&mutex_frog);
    }
    pthread_exit(NULL);

}


void *logs_move( void * t){
    /*
     * Three arguments in this function:
     * 1. start: The leftmost coordinate of the log.
     * 2. order: The 'y' coordinate of the log.
     * 3. dir: The move direction.
     */
    arg_struct * arguments = (arg_struct *) t;
    int start = arguments->start;
    int order = arguments->order;
    char dir = arguments->dir;
    /*  Move the logs  */
    //Assuming the speed of log is 1 char.
    while(!endgaming){
        if(dir == 'R'){
            pthread_mutex_lock(&mutex);
            map[order + 1][start] = ' ';
            for(int i = 1; i <= LOGLEN; ++i){
                map[order + 1][(start + i) % (COLUMN - 1)] = '=';
            }
            // Change the location of frog if the frog is on the log.
            if(frog.x == order + 1){
                pthread_mutex_lock(&mutex_frog);
                ++frog.y;
                if(endgaming)continue;
                map[frog.x][frog.y] = '0';
                pthread_mutex_unlock(&mutex_frog);
            }
            pthread_mutex_unlock(&mutex);
            if(start < COLUMN - 2) ++start;
            else start = 0;
            usleep(100000);
        } else if (dir == 'L'){
            pthread_mutex_lock(&mutex);
            map[order + 1][(start + LOGLEN - 1) % (COLUMN - 1)] = ' ';
            for (int i = 0; i < LOGLEN; ++i){
                map[order + 1][(start - 1 + i) % (COLUMN - 1)] = '=';
            }
            // Change the location of frog if the frog is on the log.
            if(frog.x == order + 1){
                pthread_mutex_lock(&mutex_frog);
                --frog.y;
                if(endgaming)continue;
                map[frog.x][frog.y] = '0';
                pthread_mutex_unlock(&mutex_frog);
            }
            pthread_mutex_unlock(&mutex);
            if(start >= 2) --start;
            else start = COLUMN - 1;
            usleep(100000);
        }
    }

    //Free the heap space
    delete arguments;
    pthread_exit(NULL);


}
// This function printout the map.
void printMap(char (* map)[COLUMN]){
    for(int i = 0; i <= ROW; ++i)
        puts( *(map + i) );
}

int main( int argc, char *argv[] ){
    // Init threads, locks, and the cond.
    pthread_t ptidarray[LOGNUM+1];
    pthread_t ptid_frog;
    pthread_t ptid_umpire;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_frog, NULL);
    pthread_cond_init(&cond_fail, NULL);
    // Initialize the river map and frog's starting position
    memset( map , 0, sizeof( map ) ) ;
    int i , j ;
    // Set white space (River)
    for( i = 1; i < ROW; ++i ){
        for( j = 0; j < COLUMN - 1; ++j )
            map[i][j] = ' ' ;
    }
    //Print | at the top and bottom.
    for( j = 0; j < COLUMN - 1; ++j )
        map[ROW][j] = map[0][j] = '|' ;

    // Set the starting point of frog.
    frog = Node( ROW, (COLUMN-1) / 2 ) ;
    map[frog.x][frog.y] = '0' ;

    // Set the logs:
    for( i = 1; i <= LOGNUM; ++i){
        // Use rand number to decide the leftmost location of one log.
        int start = std::rand() % (COLUMN - LOGLEN);
        for( j = 0; j < LOGLEN; ++j ){
            map[i][start + j] = '=';
        }
        //Record the starting point for each log
        startarray[i - 1] = start;

    }

    /*  Create pthreads for wood move.  */
    for ( i = 0; i < LOGNUM; ++i){
        if(i % 2 == 0){
            arg_struct * argument = new arg_struct{'L', i, startarray[i]};
            pthread_create(&ptidarray[i], NULL, logs_move, (void *) argument);
        } else {
            arg_struct * argument = new arg_struct{'R', i, startarray[i]};
            pthread_create(&ptidarray[i], NULL, logs_move, (void *) argument);
        }
    }
    // Create a pthread for the frog control.
    pthread_create(&ptid_frog, NULL, frog_move, NULL);
    // Create a pthread for the umpire.
    pthread_create(&ptid_umpire, NULL, check_status, NULL);
    // Print out the map.
    while (!endgaming){
        pthread_mutex_lock(&mutex);
        if(endgaming) break;
        // Clear the terminal before the next output.
        puts("\033[2J\033[H");
        printMap(map);
        pthread_mutex_unlock(&mutex);
        usleep(40000);
    }
    // Wait for all threads end.
    for (i = 0; i < LOGNUM; ++i){
        pthread_join(ptidarray[i], NULL);
    }
    pthread_join(ptid_frog, NULL);
    pthread_join(ptid_umpire, NULL);
    // Destroy (free) the locks and the cond.
    pthread_cond_destroy(&cond_fail);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_frog);
    return 0;
}

