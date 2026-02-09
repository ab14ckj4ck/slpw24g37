#include "player.h"

/******************************************************************
 * Main routine that contains the game loop. After the initial setup,
 * requests are made and published to the board.
 *
 * Stops if the 'exit' command is recognized or if EOF (CTRL+D) is
 * recognized, both should terminate the program properly.
 *
 * Initialize everything needed in here. Checks in this function must
 * not be relocated.
 *
 * Do NOT use any synchronization mechanisms outside of the TODO blocks.
 *
 * @param none
 *
 * @return predefined exit codesx
 */
int startplayer()
{
  initSharedMemoryObjectsPlayer();
  checkSHMPlayer();
  initMemoryMappingsPlayer();
  checkMMAPPlayer();
  initLocks();
  initProcess();
  checkBoard();

  // TODO Student START
  // init additional variables here if needed (optional)
  // wait for welcome message from board

  sem_wait(&mmaps.locks_->welcome_sem);

  // TODO Student END

  checkAndPrintResponse();

  char buffer[REQUEST_MAX_LENGTH];
  do
  {
    printf("[PLAYER] ");

    // TODO Student START
    // - use the provided function readCommand()
    // - make sure the loop properly stops if 'exit' command is entered
    //   or EOF is received
    // - make sure that for both exit scenarios types the board properly
    //   handles the terminates as well

    sem_wait(&mmaps.locks_->response_sem);

    if (readCommand(buffer, REQUEST_MAX_LENGTH)) {
      strcpy(mmaps.request_->message, buffer);
    } else {
      strcpy(mmaps.request_->message, CMD_EXIT);
      strcpy(buffer, CMD_EXIT);
    }

    sem_post(&mmaps.locks_->request_sem);
    sem_wait(&mmaps.locks_->response_sem);

    // TODO Student END

    checkAndPrintResponse();
    checkResults();

  } while (strcmp(buffer, CMD_EXIT) != 0);

  closeMemoryMappingsPlayer();
  checkCleanup();

  return 0;
}

/******************************************************************
 *  This function starts and initializes the Board process.
 *  Use the predefined variable process_id for the pid of the newly created
 *  process. You can ignore process_id in the Board process.
 *
 *  @param none
 *
 *  @return none
 */
void initProcess()
{
  if (checkSetup())
  {
    return;
  }

  // TODO Student START
  // start the Board process and load the right executable (hint: look inside build directory)
  process_id = fork();
    if (process_id == 0) {
      execl("./build/board", "board", NULL);
      exit(ERROR_REQ_BOARD);
    } else if (process_id < 0) {
      exit(ERROR_RES_PLAYER);
    }
  // TODO Student END
}

/******************************************************************
 * Initializes your shared objects of the Player and resizes them.
 * Make sure you only give the permissions the Player needs. But please
 * use MODERW, for compatibility with the testsystem.
 *
 * @param none
 *
 * @return none
 */
void initSharedMemoryObjectsPlayer()
{
    // TODO Student START

    fds.fd_shm_locks_ = shm_open(SHM_NAME_LOCKS, FLAGS_SHM_READWRITE, MODERW);
    fds.fd_shm_quizstate_ = shm_open(SHM_NAME_QUIZSTATE, FLAGS_SHM_READONLY, MODERW);
    fds.fd_shm_request_ = shm_open(SHM_NAME_REQUEST, FLAGS_SHM_READWRITE, MODERW);
    fds.fd_shm_response_ = shm_open(SHM_NAME_RESPONSE, FLAGS_SHM_READONLY, MODERW);

    if (fds.fd_shm_locks_ < 0 || fds.fd_shm_quizstate_ < 0 || fds.fd_shm_request_ < 0 || fds.fd_shm_response_ < 0) {
      exit(ERROR_RES_PLAYER);
    }
    else if (ftruncate(fds.fd_shm_locks_, sizeof(shmlocks)) < 0 || ftruncate(fds.fd_shm_request_, sizeof(shmrequest)) < 0) {
      exit(ERROR_RES_PLAYER);
    }

    // TODO Student END
}

/******************************************************************
 * Maps the shared objects to the virtual memory space of the Player
 * Don't do anything else in this function.
 *
 * @param none
 *
 * @return none
 */
void initMemoryMappingsPlayer()
{
  if (checkProgressPlayer())
  {
    return;
  }

  // TODO Student START

  mmaps.locks_ = mmap(NULL, sizeof(shmlocks), PROT_READ | PROT_WRITE, MAP_SHARED, fds.fd_shm_locks_, 0);
  mmaps.quizstate_ = mmap(NULL, sizeof(shmquizsstate), PROT_READ, MAP_SHARED, fds.fd_shm_quizstate_, 0);
  mmaps.request_ = mmap(NULL, sizeof(shmrequest), PROT_READ | PROT_WRITE, MAP_SHARED, fds.fd_shm_request_, 0);
  mmaps.response_ = mmap(NULL, sizeof(shmresponse), PROT_READ, MAP_SHARED, fds.fd_shm_response_, 0);

  if (mmaps.locks_ == MAP_FAILED || mmaps.quizstate_ == MAP_FAILED || mmaps.request_ == MAP_FAILED || mmaps.response_ == MAP_FAILED) {
    exit(ERROR_MMAP_PLAYER);
  }
  // TODO Student END
}

/******************************************************************
 * Initializes the locks of the shared object
 *
 * @param none
 *
 * @return none
 */
void initLocks()
{
  // TODO Student START

  // Initialisierung des Semaphors
  sem_init(&mmaps.locks_->welcome_sem, 1, 0);
  sem_init(&mmaps.locks_->request_sem, 1, 0);
  sem_init(&mmaps.locks_->response_sem, 1, 0);

  // TODO Student END
}


/******************************************************************
 * Removes all mmaps and shared objects as seen from the Player
 * This part is an essential function for closing this application
 * accordingly without leaving artifacts on your system!
 *
 * @param none
 *
 * @return none
 */
void closeMemoryMappingsPlayer()
{
  if(checkProgress())
  {
    return;
  }

  // TODO Student START

  sem_destroy(&mmaps.locks_->welcome_sem);
  sem_destroy(&mmaps.locks_->request_sem);
  sem_destroy(&mmaps.locks_->response_sem);

  munmap(mmaps.locks_, sizeof(shmlocks));
  munmap(mmaps.quizstate_, sizeof(shmquizsstate));
  munmap(mmaps.request_, sizeof(shmrequest));
  munmap(mmaps.response_, sizeof(shmresponse));

  close(fds.fd_shm_locks_);
  close(fds.fd_shm_quizstate_);
  close(fds.fd_shm_request_);
  close(fds.fd_shm_response_);

  shm_unlink(SHM_NAME_LOCKS);
  shm_unlink(SHM_NAME_REQUEST);

  waitpid(process_id, NULL, 0);

  // TODO Student END
}

/******************************************************************
 * This function is used to check the response from the Board and print it.
 *
 * @param none
 *
 * @return none
 */
void checkAndPrintResponse(void) {
  printf("[BOARD] %s\n", mmaps.response_->message);
}
