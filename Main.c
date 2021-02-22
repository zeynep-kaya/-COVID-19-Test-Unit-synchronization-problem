#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>


void *person(void *num);
void *kill();
void print();
void *room(void *num);
#define PERSON_NUMBER 24
#define ROOM_NUMBER 8
#define ROOM_CAPACITY 3

    pthread_t roomThreads[ROOM_NUMBER];
    pthread_t personThreads[PERSON_NUMBER];

sem_t rooms[ROOM_NUMBER];
sem_t person_sem; 
sem_t room_sem;
sem_t roomCap_sem;
sem_t mutex;
sem_t staff_work_time;


int numOfPerson[ROOM_NUMBER]= {0,0,0,0,0,0,0,0};
int numOfTest[ROOM_NUMBER] = {0,0,0,0,0,0,0,0}; //starvation
int statesOfRooms[ROOM_NUMBER] = {0,0,0,0,0,0,0,0};; // 0-idle, 1-entryfree, 2-fullandtest
int statesOfPersons[PERSON_NUMBER];//0- dont come or in garden, 1-in room, 2-leave(??)
int numOfRemainPerson = PERSON_NUMBER;
int atTheHospital=0;
int roomid =1;
int allDone;

void *person(void *num){
    int personid = (int)num;
    int i;
    sem_wait(&mutex); //PERSON CRITIC AREA
    
    printf("----%d arrived to hospital.\n",personid);
    statesOfPersons[personid]=0;
    sleep(random() % 2);

  
    //PEOPLE ARRIVED HOSPITAL
    sem_wait(&roomCap_sem); //ROOM CAPACITY OPEN
    sem_post(&staff_work_time); // STAFF WORK
    if(numOfPerson[roomid] == 0){ //EMPTY ROOM
        printf("Please inform the room staff at check-in!\n");
        printf("The staff warned. Ventilating is over..\n");
        printf("----The person %d entered the room %d\n",personid, roomid);
        statesOfRooms[roomid]=1;
    }
    
    numOfPerson[roomid] ++;
    print();

    sem_post(&mutex); //PERSON CRITIC AREA

}


void *room(void *num){
    int i;
    sem_wait(&person_sem);
    
    while(!allDone){

        sem_wait(&staff_work_time);
        if(!allDone){

            sem_wait(&room_sem);

            if(numOfPerson[roomid] == 3){
                statesOfRooms[roomid] =2;
            }

            int stateR= statesOfRooms[roomid];
            if(stateR==0){ //ventilating time
                printf("The room %d is being ventilated. \n",roomid);
            }
            else if(stateR==1){//entry free
                if(ROOM_CAPACITY-numOfPerson[roomid]!= 0){
                    printf("The last %d person for %d. room, let's start!\n",ROOM_CAPACITY-numOfPerson[roomid],roomid);
                }
                else if (ROOM_CAPACITY-numOfPerson[roomid]== 0){
                    statesOfRooms[roomid] = 2;
                }
            
            }
            else if(stateR==2){//full state
                printf(" ROOM %d IS FULL. TEST WILL START..\n",roomid);
                numOfRemainPerson = numOfRemainPerson - numOfPerson[roomid];

                printf(" TEST IS FINISH!\n");


                print();

                numOfPerson[roomid]=0;
                numOfTest[roomid]++;
                statesOfRooms[roomid] = 0;

                for(i=0;i<ROOM_CAPACITY;i++){
                    sem_post(&roomCap_sem);
                }
                    
                if(numOfRemainPerson!=0){
                    printf("\n%d kişi kaldı..\n\n",numOfRemainPerson);
                }

                roomid= (int*)num;
            }
            sem_post(&room_sem);
        }
    
    }

    for (i = 0; i < PERSON_NUMBER; i++) {
        sem_post(&person_sem);//stop the code
    }
    return 0;
}

void print(){
    int i,j;
    printf("\n");
    for(i=0;i<ROOM_NUMBER;i++){
        printf("ROOM %d: [",i);
        for(j=0;j<numOfPerson[i];j++){
            printf(" # ");
        }
        for(j=0;j<ROOM_CAPACITY-numOfPerson[i];j++){
            printf(" - ");
        }
        printf("]\n");
    }    
    printf("\n");
}

    
int main(){
    int i=0;

    for(i=0;i<PERSON_NUMBER;i++) {  // thread creation for each room
       statesOfPersons[i] = 0; //not in the room state
    }

    ///initialize the semaphores
    sem_init(&person_sem,0,PERSON_NUMBER);

    sem_init(&roomCap_sem,0,ROOM_CAPACITY);
    sem_init(&mutex,0,1);
    sem_init(&room_sem,0,1);
    sem_init(&staff_work_time,0,0);
    for(i=0;i<ROOM_NUMBER;i++){
        sem_init(rooms+i,0,1);
    }

    for(i=0;i<ROOM_NUMBER;i++) {  // thread creation for each room
       pthread_create(&roomThreads[i], NULL, &room, (void*)(i+1));

    }

    for(i=0;i<PERSON_NUMBER;i++) {  // thread creation for each person
       pthread_create(&personThreads[i], NULL, &person, (void*)(i+1));
    }

 
    kill();
    return 0;
}

void *kill(){ 
    int i;
    for(i=0;i<PERSON_NUMBER;i++) {
      pthread_join(personThreads[i], NULL);
    }

    for (i = 0; i < ROOM_NUMBER; i++) {
        sem_post(&staff_work_time);
    }

    allDone = 1; // to stop

    for(i=0;i<ROOM_NUMBER;i++) {
       pthread_join(roomThreads[i], NULL);
    }

   
    system("PAUSE"); 
}

