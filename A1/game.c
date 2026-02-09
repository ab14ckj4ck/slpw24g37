/*
 * Implement the missing functionality in this file. DO NOT CHANGE ANY OTHER FILE!
 * Add your code only inside the given TODO borders
 */

#include "game.h"
#include <stdlib.h>

WINDOW *game_window;
char game_map[MAP_LENGTH][MAP_WIDTH];

int number_enemies = 7;
int number_obstacles = 5;

entity_info entity_infos;

time_t time1;

int lifepoints = 100;
int points = 0;

int is_collected = 0;

entity_info* getEntityInfo(int entity_type, int entity_id)
{
    entity_info * current_node = &entity_infos;
    while(current_node != NULL)
    {
        // TODO:
        // Iterate through the linked list to find and return the entry with the same type **AND** same id
        // Make sure to update the current node!
        if (current_node->entity_type == entity_type && current_node->entity_id == entity_id) {
            return current_node;
        }

        current_node = current_node->next;

        // TODO END
    }

    return NULL;
}

entity_info* getLocalPlayer()
{
    return getEntityInfo(TYPE_PLAYER, 0);
}

entity_info* getCollectible()
{
    return getEntityInfo(TYPE_COLLECTIBLE, 0);
}

void *playerLogic()
{
    entity_info* local_player = getLocalPlayer();

    game_map[local_player->pos.y_][local_player->pos.x_] = (char)TYPE_PLAYER;

    char direction = 'l';
    char prev_direction = 'l';

    int c;
    keypad(stdscr, TRUE);
    noecho();
    timeout(1);
    while (true)
    {
        c = getch();
        switch (c)
        {
            case 's':
                direction = 'd';
                break;
            case 'w':
                direction = 'u';
                break;
            case 'a':
                direction = 'l';
                break;
            case 'd':
                direction = 'r';
                break;
            default:
                break;
        }
        if (c == 'q')
        {
            lifepoints = 0;
            continue;
        }

        usleep(100000);

        // TODO (7):
        // - If the player is on a collectible, collect it. (Hint: getMapValue, TYPE_COLLECTIBLE)
        // - If the collectible was collected, the points should be increased and the collectible should disappear. (the collectible thread should somehow terminate)
        // - also set is_collected to 1

        unsigned char map_val = getMapValue(local_player->pos.x_, local_player->pos.y_);
        if (map_val == TYPE_COLLECTIBLE) {
            points += POINTS;
            is_collected = 1;
            entity_info* coll = getCollectible();
            pthread_cancel(coll->tid);
            pthread_join(coll->tid, &coll->return_value);
        }
        // TODO END

        moveAndDrawPlayer(&direction, &prev_direction, &local_player->pos, TYPE_PLAYER);
    }
}

void *enemyObstacle(parameters *params)
{
    if (params->type_ != TYPE_ENEMY_OBSTACLE)
        return (void *)-1;

    // TODO (3):
    // Spawn the obstacle correctly in the map. The given position gives the position of the top left field.
    // Make sure to initialize the other fields correctly!
    // drawEnemy *only* initializes *one* field on the map.

    unsigned char x = params->pos_x_;
    unsigned char y = params->pos_y_;

    drawEnemy(TYPE_ENEMY_OBSTACLE, x, y);
    drawEnemy(TYPE_ENEMY_OBSTACLE, x + 1, y);
    drawEnemy(TYPE_ENEMY_OBSTACLE, x, y + 1);
    drawEnemy(TYPE_ENEMY_OBSTACLE, x + 1, y + 1);

    free(params);

    while (lifepoints > 0)
    {
        usleep(300000);
    }
    // TODO END

    return (void *)999;
}

void *enemyBasic(parameters *params)
{
    if (params->type_ != TYPE_ENEMY_BASIC)
        return (void *)-1;

    // TODO (4):
    // -) set the enemy position correctly
    // -) spawn the enemy correctly in the map

    position enemy_pos = {params->pos_x_, params->pos_y_};
    drawEnemy(TYPE_ENEMY_BASIC, params->pos_x_, params->pos_y_);
    free(params);

    // TODO END

    int nr_direction = 0;
    char prev_direction = 'l';

    while (lifepoints > 0)
    {
        usleep(300000);

        if (rand() % 5 == 0)
        {
            nr_direction = rand() % 4;
        }
        char direction;
        switch (nr_direction)
        {
            case 0:
                direction = 'l';
                moveAndDrawPlayer(&direction, &prev_direction, &enemy_pos, (char) TYPE_ENEMY_BASIC);
                break;
            case 1:
                direction = 'r';
                moveAndDrawPlayer(&direction, &prev_direction, &enemy_pos, (char) TYPE_ENEMY_BASIC);
                break;
            case 2:
                direction = 'u';
                moveAndDrawPlayer(&direction, &prev_direction, &enemy_pos, (char) TYPE_ENEMY_BASIC);
                break;
            case 3:
                direction = 'd';
                moveAndDrawPlayer(&direction, &prev_direction, &enemy_pos, (char) TYPE_ENEMY_BASIC);
                break;
            default:
                break;
        }
    }
    return (void *)998;
}

void init_enemies(unsigned char type, int number_of_enemy_type)
{
    pthread_attr_t enemy_attr;
    pthread_attr_init(&enemy_attr);

    // TODO (2): Implement the whole function:
    // - spawn all enemies of the given type (one thread per entity), make sure each one appears one it's random coordinates and has the type specified in type_
    // - use the attribute for thread creation
    // - use corresponding tid field inside entity_infos to store the tids (getEntityInfo)
    // - all enemies have to move on their own and concurrently with the others!
    // - look at the provided data structures in the header. Use them!
    // - make sure to get a new random position for each enemy entity!
    // - Arguments needed? generate them via getRandPosX() and getRandPosY()
    // - pay attention to not call the getRandPos-functions unnecessary or more often then you need, since this will fail on the testsystem
    // - Furthermore, make sure to keep the lifetime of variables in mind and to not leak any memory!

    pthread_attr_setdetachstate(&enemy_attr, PTHREAD_CREATE_JOINABLE);

    for (int enemy_index = 0; enemy_index < number_of_enemy_type; enemy_index++) {
        entity_info* enemy = getEntityInfo(type, enemy_index);
        enemy->entity_type = type;
        enemy->pos.x_ = getRandPosX();
        enemy->pos.y_ = getRandPosY();

        parameters* en_param = (parameters*)calloc(1, sizeof(parameters));
        en_param->type_ = enemy->entity_type;
        en_param->pos_y_ = enemy->pos.y_;
        en_param->pos_x_ = enemy->pos.x_;

        if (en_param->type_ == TYPE_ENEMY_BASIC) {
            pthread_create(&enemy->tid, &enemy_attr, (void *(*) (void *) ) enemyBasic, en_param);
        } else if (en_param->type_ == TYPE_ENEMY_OBSTACLE) {
            pthread_create(&enemy->tid, &enemy_attr, (void*(*) (void*)) enemyObstacle, en_param);
        }
    }

    // TODO END

    pthread_attr_destroy(&enemy_attr);
}

void *placeCollectible(parameters *params)
{
    if (params->type_ != TYPE_COLLECTIBLE)
        return (void *)-1;

    // TODO (6):
    // - In general there should be only one collectible on the map at a time
    // - place the collectible on the position taken from the function parameters
    // - the collectible has to disappear immediately if it gets collected. Think about what mechanism/characteristic which could be used for this.

    unsigned char position_x = params->pos_x_;
    unsigned char position_y = params->pos_y_;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // TODO END

    while (1)
    {
        drawCollectible(position_x, position_y);
    }
}

int start_game(int argc, char* argv[])
{
    init_screen();
    srand((unsigned int)time(&time1));

    entity_info* current_node = &entity_infos;
    memset(current_node, 0, sizeof(entity_info));

    // TODO:
    // Initialize the linked list of the local player and enemies
    // Make sure to set the correct type and id
    // IDs only have to be unique per typ
    // For simplicity, you can use the root current node as the local player.
    // Just make sure to use the ID 0 for the local player as well as the correct type.
    {
        current_node->entity_type = TYPE_PLAYER;
        current_node->entity_id = 0;
    }

    for(int i = 0; i < number_enemies + number_obstacles; i++)
    {
        
        // TODO: Allocate a new node and update the linked list with the new node
        // Don't forget to update current_node
        entity_info* new_node = (entity_info*)calloc(1, sizeof(entity_info));
        current_node->next = new_node;
        current_node = current_node->next;
        // TODO END

        // TODO: Update the enemy type and id
        // The id **must** only be unique per type. For simplicity, we recommend this approach
        // e.g the first obstacle has id 0 and the first basic enemy has id 0

        // However, feel free to use unique ids if you want to
        // Depending on how you choose your IDs, make sure to account for this when implementing other TODOs
        if (i < number_enemies) {
            current_node->entity_id = i;
            current_node->entity_type = TYPE_ENEMY_BASIC;
        }
        else {
            current_node->entity_id = i - number_enemies;
            current_node->entity_type = TYPE_ENEMY_OBSTACLE;
        }
        // TODO END
    }

    init_map();

    // TODO (1):
    // - spawn the player and enemies by creating separate threads for each entity
    // - Arguments needed? Attributes needed? Look at the playerLogic and the enemy functions
    // - look at the pthread_t variables in the entity structs, make sure to use them.
    // - Init all basic enemies with one call to init_enemies and all obstacles with another call to init_enemies
    // - Generate number_obstacles obstacles and number_enemies enemies
    // - Make sure to generate all enemies first, then all obstacles

    entity_info* local_player = getLocalPlayer();

    local_player->pos.x_ = MAP_WIDTH / 2;
    local_player->pos.y_ = MAP_LENGTH - 2;

    pthread_create(&local_player->tid, NULL, playerLogic, NULL);
    init_enemies(TYPE_ENEMY_BASIC, number_enemies);
    init_enemies(TYPE_ENEMY_OBSTACLE, number_obstacles);

    // TODO END

    refreshMap();

    // TODO (5):
    // - spawn an initial collectible by adding it to the entity_infos and creating a collectible thread (for position call getRandPosX() and getRandPosY())
    // - if a collectible is collected (there's a flag for that), a new collectible shall be spawned at a new random position
    // - reset the collected flag ONLY AFTER you spawned a new collectible
    // - pay attention to not call the getRandPos-functions unnecessary or more often then you need, since this will fail on the testsystem
    // - make sure that all collectible threads are terminated and store the return value inside the struct
    // - have a look at the other TODOs from above - there might be interconnects
    // - Arguments needed? generate them via getRandPosX() and getRandPosY()

    entity_info* coll = (entity_info*)calloc(1, sizeof(entity_info));
    coll->pos.x_ = getRandPosX();
    coll->pos.y_ = getRandPosY();
    coll->entity_type = TYPE_COLLECTIBLE;
    coll->entity_id = 0;
    coll->next = NULL;

    while (current_node->next != NULL) {
      current_node = current_node->next;
    }
    current_node->next = coll;

    parameters* c_pa = (parameters*)calloc(1, sizeof(parameters));
    c_pa->type_ = coll->entity_type;
    c_pa->pos_y_ = coll->pos.y_;
    c_pa->pos_x_ = coll->pos.x_;

    pthread_create(&coll->tid, NULL, (void *(*)(void *))placeCollectible, c_pa);

    // TODO END

    while (lifepoints > 0)
    {
        usleep(10000);

        if (is_collected == 1)
        {
            // TODO (8): Further implementations from TODO above
            if (c_pa != NULL) free(c_pa);
            c_pa = NULL;

            c_pa = (parameters*)calloc(1, sizeof(parameters));
            c_pa->type_ = TYPE_COLLECTIBLE;
            c_pa->pos_y_ = getRandPosY();
            c_pa->pos_x_ = getRandPosX();

            coll->pos.x_ = c_pa->pos_x_;
            coll->pos.y_ = c_pa->pos_y_;

            pthread_create(&coll->tid, NULL, (void *(*) (void*)) placeCollectible, c_pa);

            is_collected = 0;
            // TODO END
        }


      refreshMap();
    }
  if (lifepoints <= 0 && c_pa != NULL) {
    free(c_pa);
    c_pa = NULL;
  }

    // TODO (9):
    // - make sure that all the running threads are terminated before returning from main and fetch their return values
    // - All threads which are able to return on their own should also do that so and therefore give back the corresponding return value
    // - Force the termination of threads which do not terminate on their own.
    // - have a closer look on the other TODOs. There might be some connections
    // - we want to make sure, that all threads are terminated for sure. But are all threads even joinable?
    //   Maybe the tutor made a stupid mistake somewhere in the upstream code. Fix it.
    // - Store the return values in the corresponding field inside entity_info (getEntityInfo)

    current_node = local_player;

    while (current_node != NULL) {
      entity_info* next_node = current_node->next;

        if (current_node->tid) {
            if (current_node->entity_type == TYPE_PLAYER || current_node->entity_type == TYPE_COLLECTIBLE) pthread_cancel(current_node->tid);
            pthread_join(current_node->tid, &current_node->return_value);
        }
      current_node = next_node;
    }

    // TODO END

    return end_game();
}