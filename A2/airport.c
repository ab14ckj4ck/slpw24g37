#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "definitions.h"

// ------------------------------------------------------------------------
// STUDENT : define synchronization primitives here
sem_t check_in_line_opened_sem; // employee
sem_t wait_for_passenger_in_queue_sem; // employee
sem_t self_check_in_reset_sem; // self-check-in
sem_t shut_down_sem; // self-check-in
sem_t wait_for_self_check_in_sem;
sem_t self_check_in_maintenance_sem;

pthread_mutex_t queue_lock; // employee
pthread_mutex_t employee_waiting_for_queue_lock; // employee
pthread_mutex_t find_self_check_in_lock; // passenger

sem_t check_in_line_global_opened_sem;

pthread_mutex_t check_in_line_opened_lock;
pthread_mutex_t check_in_completed_lock;

pthread_mutex_t print_lock;
pthread_mutex_t no_more_passengers_lock;
pthread_mutex_t vector_edit_lock;
pthread_mutex_t options_lock;
pthread_mutex_t baggage_lock;

// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
/* STUDENT :
 * -) Ensure that potential shared resources are properly locked to prevent data races!
 * -) Implement appropriate synchronization strategies to avoid thread starvation!
 * HINT: Initially, ignore the baggage check-in part and focus on getting the rest working correctly.
 *       Then, integrate the baggage check-in while taking care of both - synchronization and performance.
 *       Consider the following:
 *       - Does it make sense for one passenger to wait until another finishes checking in,
 *         if there are free spots available?
 *       - What is the maximum number of passengers that can check in simultaneously?
 *       - Should one passenger dropping off baggage prevent another from searching for
 *         a free baggage check-in counter?
 */
void checkInBaggage(Passenger *passenger) {
    int i = 0;

    while (baggage_drop_counters[i] != NULL) {
        i++;
    }

    baggage_drop_counters[i] = passenger;

    dropOffBaggage();

    baggage_drop_counters[i] = NULL;
}

// ------------------------------------------------------------------------
/* STUDENT :
 * Ensure that potential shared resources are properly locked to prevent data races!
 */

vector_iterator findFreeSelfCheckIn() {
    pthread_mutex_lock(&vector_edit_lock);
    vector_iterator it = vector_begin(&free_self_check_in_machines);
    vector_iterator it_end = vector_end(&free_self_check_in_machines);
    pthread_mutex_unlock(&vector_edit_lock);

    for (; it != it_end; ++it) {
        SelfCheckIn *self_check_in = (SelfCheckIn *) *it;
        pthread_mutex_lock(&self_check_in->state_lock);
        if (self_check_in->state != OUT_OF_SERVICE) {
            pthread_mutex_unlock(&self_check_in->state_lock);
            return it;
        }
        pthread_mutex_unlock(&self_check_in->state_lock);
    }

    return NULL;
}

// ------------------------------------------------------------------------
/* STUDENT :
 * Ensure that potential shared resources are properly locked to prevent data races!
 */
vector_iterator findSelfCheckInToBeMaintained() {
    pthread_mutex_lock(&vector_edit_lock);
    vector_iterator it = vector_begin(&free_self_check_in_machines);
    vector_iterator it_end = vector_end(&free_self_check_in_machines);
    pthread_mutex_unlock(&vector_edit_lock);

    for (; it != it_end; ++it) {
        SelfCheckIn *self_check_in = (SelfCheckIn *) *it;
        pthread_mutex_lock(&self_check_in->state_lock);
        if (self_check_in->state == OUT_OF_SERVICE && self_check_in->state != OFF) {
            pthread_mutex_unlock(&self_check_in->state_lock);
            return it;
        }
        pthread_mutex_unlock(&self_check_in->state_lock);
    }

    return NULL;
}

// ------------------------------------------------------------------------
/* STUDENT :
 * Ensure that shared resources are properly locked to prevent data races!
 *
 * HINT PART 1: Start by focusing on the staffed check-in process, ignoring self-check-in at first.
 *              Use appropriate synchronization primitives to ensure the following:
 *              -) No passenger should enter the waiting queue before the first employee arrives (check_in_line_opened = true).
 *              -) Employees should be notified as soon as new passengers arrive.
 *              -) No passenger should proceed to the gate before completing the check-in process.
 *
 * HINT PART 2: After the staffed check-in is functioning correctly, shift focus to the self-check-in process.
 *              Ensure the following with proper synchronization:
 *              -) Every passenger must be able to find an available self-check-in.
 *              -) Ensure that the self-checkin/passenger interaction adheres to the assignment description and follow-up comments.
 *              -) Prevent two passengers from selecting the same self-check-in at the same time.
 *              -) Determine the maximum number of passengers that can self-check in simultaneously.
 *              -) Should a passenger wait for another to finish if a self-check-in is available?
 */
void passenger(Passenger *passenger) {
    // 1) Passenger takes decision - "staffed check-in lane"
    pthread_mutex_lock(&options_lock);
    if (!useSelfCheckIn()) {
        pthread_mutex_unlock(&options_lock);
        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Passenger %zd goes to the check-in lane\n", passenger->id);
        pthread_mutex_unlock(&print_lock);

        // 1a) Wait for the check-in lane to open
        sem_wait(&check_in_line_opened_sem);

        pthread_mutex_lock(&check_in_line_opened_lock);
        if (!check_in_line_opened) {
            pthread_mutex_lock(&print_lock);
            printf("[OOPS] Passenger %zd appears to be early for their check-in! The lane hasn't opened yet!! \n", passenger->id);
            pthread_mutex_unlock(&print_lock);
        }
        pthread_mutex_unlock(&check_in_line_opened_lock);

        // 1b) Wait in the queue
        pthread_mutex_lock(&queue_lock);
        WaitingPassenger waiting_passenger = {passenger};
        queue_push_back(waiting_passenger);
        pthread_mutex_unlock(&queue_lock);

        sem_post(&wait_for_passenger_in_queue_sem);

        // 1c) Wait for the check-in process to be completed
        sem_wait(&passenger->passenger_employee_sem);
        pthread_mutex_lock(&check_in_completed_lock);
        if (!passenger->check_in_completed) {
            pthread_mutex_lock(&print_lock);
            printf("[OOPS] Keep calm and wait for the check-in to complete @Customer %zd!!\n", passenger->id);
            pthread_mutex_unlock(&print_lock);
        }
        pthread_mutex_unlock(&check_in_completed_lock);
        // 3) Passenger can go once check-in completed
        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Passenger %zd can head to the gate now\n", passenger->id);
        pthread_mutex_unlock(&print_lock);
        return;
    }

    // 2) Passenger takes decision - using a self-check-in machines
    pthread_mutex_unlock(&options_lock);
    sem_post(&shut_down_sem);
    pthread_mutex_lock(&print_lock);
    printf("[ OK ] Passenger %zd is looking for an available self-check-in\n", passenger->id);
    pthread_mutex_unlock(&print_lock);

    // 2a) Find an available self-service kiosk in the self-service lane
    sem_wait(&self_check_in_reset_sem); //TODO Wartet darauf, dass die SelfCheckIn wieder im Urzustand ist
    pthread_mutex_lock(&find_self_check_in_lock);
    vector_iterator self_checkin_it = findFreeSelfCheckIn();
    SelfCheckIn *self_check_in = (SelfCheckIn *) *self_checkin_it;

    if (self_check_in->passenger) {
        pthread_mutex_lock(&print_lock);
        printf("[OOPS] Oh, someone is excited before the yourney! Please, wait for your turn @Customer %zd!\n", passenger->id);
        pthread_mutex_unlock(&print_lock);
    }

    self_check_in->passenger = passenger;
    pthread_mutex_lock(&vector_edit_lock);
    vector_erase(&free_self_check_in_machines, self_checkin_it);
    pthread_mutex_unlock(&vector_edit_lock);
    pthread_mutex_unlock(&find_self_check_in_lock);

    // 2b) Initiate the interaction with the self-check-in (state = START)
    pthread_mutex_lock(&self_check_in->state_lock);
    pthread_mutex_lock(&print_lock);
    assert(self_check_in->state == READY && "Ohh, something has gone haywire...?!"); // consider using asserts for debugging ;)
    pthread_mutex_unlock(&print_lock);
    pthread_mutex_unlock(&self_check_in->state_lock);
    pthread_mutex_lock(&self_check_in->state_lock);
    self_check_in->state = START;
    pthread_mutex_unlock(&self_check_in->state_lock);
    sem_post(&self_check_in->self_check_in_sem); //TODO Hier wird der Self-Check-In gestartet und SelfCheckIn darf zu ID Scan weiter gehen

    // 2c) Scan the id as soon as the instruction appears on the screen (state = ID_SCAN)
    sem_wait(&passenger->passenger_self_check_in_sem); //TODO Wartet darauf, dass der Self-Check-In zu ID Scan weiter geht
    pthread_mutex_lock(&self_check_in->state_lock);
    if (self_check_in->state == ID_SCAN) {
        pthread_mutex_unlock(&self_check_in->state_lock);
        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Passenger %zd can scan the id now.\n", passenger->id);
        pthread_mutex_unlock(&print_lock);

        scanID();

        // 2d) Identification completed (state = PASSENGER_IDENTIFIED)
        pthread_mutex_lock(&self_check_in->state_lock);
        self_check_in->state = PASSENGER_IDENTIFIED;
        pthread_mutex_unlock(&self_check_in->state_lock);
        sem_post(&self_check_in->self_check_in_sem); //TODO Sagt dem Self-Check-In, dass der Passagier identifiziert wurde
    }

    // 2e) Wait for the approvement
    sem_wait(&passenger->passenger_self_check_in_sem); //TODO Wartet darauf, dass der Self-Check-In zu Print Ticket weiter geht
    assert(self_check_in->passenger == passenger);

    pthread_mutex_lock(&self_check_in->state_lock);
    if (self_check_in->state != PRINT_TICKET) {
        pthread_mutex_lock(&print_lock);
        printf("[OOPS] Hey, passenger %zd! Where do you think you're going without a ticket?\n", passenger->id);
        pthread_mutex_unlock(&print_lock);
    }
    pthread_mutex_unlock(&self_check_in->state_lock);

    pthread_mutex_lock(&print_lock);
    printf("[ OK ] Passenger %zd has completed check-in.\n", passenger->id);
    pthread_mutex_unlock(&print_lock);

    // 2f) drop the baggage at the express line if needed
    pthread_mutex_lock(&options_lock);
    if (hasBaggage()) {
        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Passenger %zd must drop the baggage off at the express line.\n", passenger->id);
        pthread_mutex_unlock(&print_lock);
        pthread_mutex_lock(&baggage_lock);
        checkInBaggage(passenger);
        pthread_mutex_unlock(&baggage_lock);
    }

    pthread_mutex_unlock(&options_lock);


    pthread_mutex_lock(&print_lock);
    printf("[ OK ] Passenger %zd can proceed to the gate now\n", passenger->id);
    pthread_mutex_unlock(&print_lock);
    sem_post(&self_check_in->self_check_in_sem); //TODO Sagt dem Self-Check-In, dass der Passagier fertig ist
}

// ------------------------------------------------------------------------
/* STUDENT :
 * -) Ensure shared resources are properly locked to prevent data races!
 * -) Notify waiting passengers as soon as the first employee starts working!
 * -) Use appropriate synchronization mechanisms to ensure that an employee acts only with a reason - either there is a waiting passenger,
 *    or there are no more passengers and the employee can go home. (fee free to use is_empty() as the condition if needed)
 * -) Notify the passenger once the check-in procedure is completed!
 * -) Ensure your locking approach does not introduce issues during cancellation!
 */

void employee(Employee *employee) {
    pthread_mutex_lock(&check_in_line_opened_lock);
    if (!check_in_line_opened) {
        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Employee %zd opens the check-in lane\n", employee->id);
        pthread_mutex_unlock(&print_lock);
        assert(!check_in_line_opened);
        check_in_line_opened = true;
        sem_post(&check_in_line_global_opened_sem);
    }
    pthread_mutex_unlock(&check_in_line_opened_lock);


    while (true) {
        // 1) check-in the next passenger in the waiting line or go home if no more passengers
        sem_wait(&wait_for_passenger_in_queue_sem); //TODO Wartet darauf, dass ein Passagier in der Warteschlange ist
        pthread_mutex_lock(&no_more_passengers_lock);
        if (no_more_passengers) {
            pthread_mutex_unlock(&no_more_passengers_lock);
            pthread_mutex_lock(&print_lock);
            printf("[ OK ] Employee %zd: Yeey, let me fly away now :D!!\n", employee->id);
            pthread_mutex_unlock(&print_lock);
            sem_post(&wait_for_passenger_in_queue_sem); //TODO Gibt den Mitarbeiter frei, sodass er in seine abbruch bedingung kommt
            break;
        } else {
            pthread_mutex_unlock(&no_more_passengers_lock);
        }

        pthread_mutex_lock(&queue_lock);
        WaitingPassenger waiting_passenger = queue_front();
        Passenger *passenger = waiting_passenger.passenger;
        queue_pop();
        pthread_mutex_unlock(&queue_lock);

        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Employee %zd is about to check-in the passenger %zd \n", employee->id, passenger->id);
        pthread_mutex_unlock(&print_lock);

        checkIn();

        // 2) inform passenger once proceeded and continue
        pthread_mutex_lock(&check_in_completed_lock);
        passenger->check_in_completed = true;
        pthread_mutex_unlock(&check_in_completed_lock);

        sem_post(&passenger->passenger_employee_sem); //TODO Sagt dem Passenger, dass er eingecheckt wurde
    }
}

// ------------------------------------------------------------------------
/* STUDENT :
 * -) Ensure shared resources are properly locked to prevent data races!
 * -) Ensure that your locking approach does not introduce any problems upon cancellation!
 */
void maintainer(Maintainer *maintainer) {
    while (1) {
        // 1) Check if any of the self-check-in machines needs maintenance in the self-rythmus (no notification needed)
        pthread_mutex_lock(&find_self_check_in_lock);
        vector_iterator self_checkin_it = findSelfCheckInToBeMaintained();

        // 2) Perform maintenance on an out-of-service self-check-in
        if (self_checkin_it) {
            SelfCheckIn *self_check_in = (SelfCheckIn *) *self_checkin_it;
            pthread_mutex_lock(&vector_edit_lock);
            vector_erase(&free_self_check_in_machines, self_checkin_it);
            pthread_mutex_unlock(&vector_edit_lock);
            pthread_mutex_unlock(&find_self_check_in_lock);

            pthread_mutex_lock(&print_lock);
            printf("[ OK ] Maintainer %zd is performing maintenance on self-service %zd.\n", maintainer->id, self_check_in->id);
            pthread_mutex_unlock(&print_lock);

            maintenance();

            // 2a) After maintenance the self-check-in is again ready for the next passenger
            pthread_mutex_lock(&self_check_in->state_lock);
            self_check_in->state = READY;
            pthread_mutex_unlock(&self_check_in->state_lock);

            // 2b) Ensure the self-check-in is again available for active usage
            pthread_mutex_lock(&vector_edit_lock);
            vector_push_back(&free_self_check_in_machines, self_check_in);
            pthread_mutex_unlock(&vector_edit_lock);

            sem_post(&self_check_in_reset_sem);
            sem_post(&self_check_in_maintenance_sem);
            continue;
        }

        pthread_mutex_unlock(&find_self_check_in_lock);
        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Maintainer %zd has nothing to do...\n", maintainer->id);
        pthread_mutex_unlock(&print_lock);
        coffeeTime();
        continue;
    }
}

// ------------------------------------------------------------------------
/* STUDENT :
 * - Ensure shared resources are properly locked to prevent data races.
 * - Verify that the self-check/passenger interaction adheres to the assignment description and follow-up comments.
 * - Explore an appropriate termination condition to allow threads to exit cleanly.
 * - Ensure your locking approach does not cause issues with the state logic.
 */

void selfCheckIn(SelfCheckIn *self_check_in) {
    while (true) {
        pthread_mutex_lock(&self_check_in->state_lock);
        if (self_check_in->state == OUT_OF_SERVICE) {
            pthread_mutex_unlock(&self_check_in->state_lock);
            sem_wait(&self_check_in_maintenance_sem);
        } else {
            pthread_mutex_unlock(&self_check_in->state_lock);
        }

        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Self-Check-In %zd: Welcome!! Please press Start!!\n", self_check_in->id);
        pthread_mutex_unlock(&print_lock);

        // 1) Wait for the start action (==> passenger assignment + state = START or ==> closing - state = OFF)
        sem_wait(&shut_down_sem);
        pthread_mutex_lock(&no_more_passengers_lock);
        if (no_more_passengers) {
            pthread_mutex_unlock(&no_more_passengers_lock);
            sem_wait(&self_check_in->self_check_in_kill_sem);
        } else {
            pthread_mutex_unlock(&no_more_passengers_lock);
        }

        pthread_mutex_lock(&self_check_in->state_lock);
        if (self_check_in->state == OFF) {
            pthread_mutex_unlock(&self_check_in->state_lock);
            sem_post(&self_check_in_maintenance_sem);
            sem_post(&self_check_in->self_check_in_sem);
            sem_post(&self_check_in->self_check_in_kill_sem);
            sem_post(&shut_down_sem);
            break;
        } else {
            pthread_mutex_unlock(&self_check_in->state_lock);
        }
        sem_wait(&self_check_in->self_check_in_sem); //TODO Wartet darauf, dass der Passagier den Self-Check-In startet

        pthread_mutex_lock(&self_check_in->state_lock);
        if (self_check_in->state != START) {
            pthread_mutex_lock(&print_lock);
            printf("[OOPS] Self-Check-In %zd may need maintenance! Proceeding without new passenger! \n", self_check_in->id);
            pthread_mutex_unlock(&print_lock);
        }
        pthread_mutex_unlock(&self_check_in->state_lock);

        // 2) Switch to the ID_SCAN state and allow the passenger to proceed
        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Self-Checkout %zd: Please scan your pass/ID according to the instructions on the screen!\n", self_check_in->id);
        pthread_mutex_unlock(&print_lock);
        pthread_mutex_lock(&self_check_in->state_lock);
        self_check_in->state = ID_SCAN;
        pthread_mutex_unlock(&self_check_in->state_lock);
        sem_post(&self_check_in->passenger->passenger_self_check_in_sem); //TODO Sagt dem Passagier, dass er scannen kann

        // 3) Wait for the passenger to scan the pass and proceed
        sem_wait(&self_check_in->self_check_in_sem); //TODO Wartet darauf, dass der Passagier identifiziert wird
        pthread_mutex_lock(&self_check_in->state_lock);
        if (self_check_in->state != PASSENGER_IDENTIFIED) {
            pthread_mutex_lock(&print_lock);
            printf("[OOPS] Self-Checkout %zd: The passenger may be experiencing issues while scanning their ID...\n", self_check_in->id);
            pthread_mutex_unlock(&print_lock);
        }
        pthread_mutex_unlock(&self_check_in->state_lock);

        printTicket();

        pthread_mutex_lock(&print_lock);
        printf("[ OK ] Self-Service %zd: Thanks for flying with us! Your ticket is being printed! Please, drop out your baggage at our express line\n", self_check_in->id);
        pthread_mutex_unlock(&print_lock);
        pthread_mutex_lock(&self_check_in->state_lock);
        self_check_in->state = PRINT_TICKET;
        pthread_mutex_unlock(&self_check_in->state_lock);
        sem_post(&self_check_in->passenger->passenger_self_check_in_sem); //TODO Sagt dem Passagier, dass er das Ticket hat

        // 4) Switch the state = READY <<-- can deal with a new passenger
        sem_wait(&self_check_in->self_check_in_sem); //TODO Wartet darauf, dass der Passagier den Self-Check-In verlässt
        self_check_in->passenger = NULL;
        pthread_mutex_lock(&self_check_in->state_lock);
        self_check_in->state = READY;
        pthread_mutex_unlock(&self_check_in->state_lock);
        pthread_mutex_lock(&vector_edit_lock);
        vector_push_back(&free_self_check_in_machines, self_check_in);
        pthread_mutex_unlock(&vector_edit_lock);

        // 6) Unfortunately, some self-checki-in machines may be out of service from time to time  !! watch out here !!
        pthread_mutex_lock(&options_lock);
        if (outOfService()) {
            pthread_mutex_unlock(&options_lock);
            pthread_mutex_lock(&self_check_in->state_lock);
            self_check_in->state = OUT_OF_SERVICE;
            pthread_mutex_unlock(&self_check_in->state_lock);
            pthread_mutex_lock(&print_lock);
            printf("[ OK ] Self-Service %zd: I will be back soon! Thanks for your patiance!! \n", self_check_in->id);
            pthread_mutex_unlock(&print_lock);
            continue;
        }
        pthread_mutex_unlock(&options_lock);
        sem_post(&self_check_in_reset_sem); //TODO Sagt dem Passenger, dass die Maschine wieder im Urzustand ist
    }
}

// ------------------------------------------------------------------------
/* STUDENT :
 * Ensure shared resources are properly locked to prevent data races.
 * Initialize and destroy mutexes, condition variables, and semaphores as needed.
 * --> You may do this in other parts of the codebase as well, but ensure synchronization primitives
 *     are initialized before their first use!
 */

int main(int argc, char *argv[]) {
    sem_init(&check_in_line_opened_sem, 0, 0);
    sem_init(&wait_for_passenger_in_queue_sem, 0, 0);
    sem_init(&self_check_in_reset_sem, 0, 1);
    sem_init(&shut_down_sem, 0, 0);
    sem_init(&wait_for_self_check_in_sem, 0, 0);
    sem_init(&self_check_in_maintenance_sem, 0, 0);

    pthread_mutex_init(&queue_lock, 0);
    pthread_mutex_init(&employee_waiting_for_queue_lock, 0);
    pthread_mutex_init(&find_self_check_in_lock, 0);

    sem_init(&check_in_line_global_opened_sem, 0, 0);
    pthread_mutex_init(&options_lock, 0);

    pthread_mutex_init(&check_in_line_opened_lock, 0);
    pthread_mutex_init(&check_in_completed_lock, 0);
    pthread_mutex_init(&print_lock, 0);
    pthread_mutex_init(&no_more_passengers_lock, 0);
    pthread_mutex_init(&vector_edit_lock, 0);
    pthread_mutex_init(&baggage_lock, 0);


    srand(time(NULL));

    // ~ arguments
    ssize_t num_passengers;
    ssize_t num_employees;
    ssize_t num_self_checkins;
    handleArguments(argc, argv, &num_passengers, &num_employees, &num_self_checkins);

    // ~ create the threads cast
    Passenger *passengers = malloc(num_passengers * sizeof(Passenger));
    Maintainer *maintainers = malloc(num_employees * sizeof(Maintainer));
    SelfCheckIn **self_chekins = malloc(num_self_checkins * sizeof(SelfCheckIn *));
    Employee *employees = malloc(num_employees * sizeof(Employee));

    if (!passengers || !employees || !self_chekins || !maintainers) {
        goto error;
    }
    pthread_mutex_lock(&vector_edit_lock);
    createSelfCheckIns(self_chekins, num_self_checkins);
    pthread_mutex_unlock(&vector_edit_lock);

    // TODO Passenger init's
    for (ssize_t i = 0; i < num_passengers; i++) {
        sem_init(&passengers[i].passenger_employee_sem, 0, 0);
        sem_init(&passengers[i].passenger_self_check_in_sem, 0, 0);
    }

    //TODO SelfCheckIn init's
    for (ssize_t i = 0; i < num_self_checkins; i++) {
        sem_init(&self_chekins[i]->self_check_in_sem, 0, 0);
        sem_init(&self_chekins[i]->self_check_in_kill_sem, 0, 0);
        pthread_mutex_init(&self_chekins[i]->state_lock, 0);
    }

    queue_init();
    pthread_mutex_lock(&options_lock);
    createEmployees(employees, num_employees, (void *(*)(void *)) employee);
    pthread_mutex_unlock(&options_lock);
    createSelfCheckMachines(self_chekins, num_self_checkins, (void *(*)(void *)) selfCheckIn);
    createPassengers(passengers, num_passengers, (void *(*)(void *)) passenger);
    pthread_mutex_lock(&options_lock);
    createMaintainers(maintainers, num_employees, (void *(*)(void *)) maintainer);
    pthread_mutex_unlock(&options_lock);

    sem_wait(&check_in_line_global_opened_sem);

    for (ssize_t i = 0; i < num_passengers; i++) {
        sem_post(&check_in_line_opened_sem);
    }

    // ~ join passangers
    for (ssize_t i = 0; i < num_passengers; i++) {
        pthread_join(passengers[i].passenger_tid, NULL);
        sem_destroy(&passengers[i].passenger_employee_sem);  //TODO Destroy Passenger locks
        sem_destroy(&passengers[i].passenger_self_check_in_sem);
    }

    pthread_mutex_lock(&no_more_passengers_lock);
    no_more_passengers = true;  // !! potential breaking condition for employees !!
    pthread_mutex_unlock(&no_more_passengers_lock);

    for (ssize_t i = 0; i < num_self_checkins; i++) {
        sem_post(&shut_down_sem);
    }

    // ~ cancel and join maintainers
    for (ssize_t i = 0; i < num_employees; i++) {
        pthread_cancel(maintainers[i].maintainer_tid);
        pthread_join(maintainers[i].maintainer_tid, NULL);
    }

    // join employees
    for (ssize_t i = 0; i < num_employees; i++) {
        sem_post(&wait_for_passenger_in_queue_sem); //TODO Gibt den Mitarbeiter frei, sodass er in seine abbruch bedingung kommt
        pthread_join(employees[i].employee_tid, NULL);
    }
    sem_destroy(&check_in_line_opened_sem);
    sem_destroy(&wait_for_passenger_in_queue_sem);
    pthread_mutex_destroy(&queue_lock);
    pthread_mutex_destroy(&employee_waiting_for_queue_lock);

    // ~ finally join the self-check-in machines, !! take care of the breaking condition !!
    for (ssize_t i = 0; i < num_self_checkins; i++) {
        sem_post(&self_chekins[i]->self_check_in_sem);
        self_chekins[i]->state = OFF;
        sem_post(&self_chekins[i]->self_check_in_kill_sem);
        sem_post(&self_check_in_maintenance_sem);
        pthread_join(self_chekins[i]->self_check_in_tid, NULL);

        sem_destroy(&self_chekins[i]->self_check_in_sem);
        sem_destroy(&self_chekins[i]->self_check_in_kill_sem);
        pthread_mutex_destroy(&self_chekins[i]->state_lock);
    }
    sem_destroy(&self_check_in_reset_sem);
    sem_destroy(&shut_down_sem);
    sem_destroy(&wait_for_self_check_in_sem);
    sem_destroy(&self_check_in_maintenance_sem);

    sem_destroy(&check_in_line_global_opened_sem);
    pthread_mutex_destroy(&options_lock);
    pthread_mutex_destroy(&check_in_line_opened_lock);
    pthread_mutex_destroy(&check_in_completed_lock);
    pthread_mutex_destroy(&no_more_passengers_lock);
    pthread_mutex_destroy(&vector_edit_lock);
    pthread_mutex_destroy(&find_self_check_in_lock);
    pthread_mutex_destroy(&baggage_lock);

    freeResources(passengers, employees, self_chekins, maintainers);

    pthread_mutex_lock(&print_lock);
    printf("[ OK ] !! T H A T ' S    A L L    F O L K S !!\n");
    printf("[ OK ] !!        B O N  V O Y A G E         !!\n");
    pthread_mutex_unlock(&print_lock);

    pthread_mutex_destroy(&print_lock);

    return 0;

    error:
    free(passengers);
    free(employees);
    free(self_chekins);
    free(maintainers);
    fprintf(stderr, "Could not allocate memory!\n");
    exit(ERROR);
}