#include<iostream>
#include <fstream>
#include<vector>
#include <random>
#include <algorithm>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // For sleep function
#include <chrono> // For time


#define NUM_PRINTING_STATIONS 4
#define NUM_BINDING_STATIONS 2

using namespace std;

chrono::steady_clock::time_point initialTime;

pthread_mutex_t printing_mutexes[NUM_PRINTING_STATIONS+1];
sem_t binding_sem;
pthread_mutex_t entry_book_mutex;
pthread_mutex_t file_output_mutex;

pthread_mutex_t *group_mutex;
int *print_completed;

int rc=0;
int num_of_submission=0;
pthread_mutex_t write_mut;
ofstream outFile;
ifstream inFile;

int N,M,w,x,y;

//random number generation
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
default_random_engine gen(seed);
// Poison distribution for generating random number
double interArrivalRate = 0.33;
poisson_distribution<int> distribution(1/interArrivalRate);
poisson_distribution<int> distribution2(10);

typedef struct
{
    int student_id;
    string status;
    sem_t sem;
} Student;
Student *students_info;

int getRand()
{
    return distribution(gen);
}

int getRand2()
{
    return distribution2(gen);
}

// Function to get the time difference in seconds
int getTime()
{
    auto currentTime = chrono::steady_clock::now();
    return round(chrono::duration<double>(currentTime - initialTime).count());
}

void check(int sid)
{
    if(students_info[sid].status=="WAITING")
    {
        int station_id = sid % NUM_PRINTING_STATIONS+1;
        bool flag = false;
        for(int i=1; i<=N; i++)
        {
            int sti = i% NUM_PRINTING_STATIONS+1;
            if(sti==station_id && students_info[i].status=="PRINTING")
            {
                flag = true;
                break;
            }
        }
        if(!flag)
        {
            students_info[sid].status="PRINTING";
            sem_post(&students_info[sid].sem);
        }
    }
}

void* student_action(void* arg)
{
    Student* student = (Student*)arg;
    int student_id = student->student_id;
    int station_id = student_id % NUM_PRINTING_STATIONS+1;
    int group_id = ceil(1.00*student_id/M);

    bool isLeader;
    if(ceil(1.00*student_id/M)==(student_id/M))
        isLeader = true;
    else
        isLeader = false;

    sleep(getRand());

    string str = "Student "+to_string(student_id)+" has arrived at the print station "+to_string(station_id)+" at time "+to_string(getTime())+"\n";
    pthread_mutex_lock(&file_output_mutex);
    outFile<<str;
    pthread_mutex_unlock(&file_output_mutex);


    //dining philosopher
    //take fork
    pthread_mutex_lock(&printing_mutexes[station_id]);
    student->status = "WAITING";
    check(student_id);
    pthread_mutex_unlock(&printing_mutexes[station_id]);
    sem_wait(&student->sem);

    str = "Student "+to_string(student_id)+" has started printing at time "+to_string(getTime())+"\n";
    pthread_mutex_lock(&file_output_mutex);
    outFile<<str;
    pthread_mutex_unlock(&file_output_mutex);

    sleep(w);

    str = "Student "+to_string(student_id)+" has finished printing at time "+to_string(getTime())+"\n";
    pthread_mutex_lock(&file_output_mutex);
    outFile<<str;
    pthread_mutex_unlock(&file_output_mutex);

    sleep(getRand()); //random sleep before submitting report to group leader

    //update group print-completed
    pthread_mutex_lock(&group_mutex[group_id]);
    print_completed[group_id]++;
    if(print_completed[group_id]==M)
    {
        str = "Group "+to_string(group_id)+" has finished printing at time "+to_string(getTime())+"\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);

        int group_leader = group_id*M;
        sem_post(&students_info[group_leader].sem);
    }
    pthread_mutex_unlock(&group_mutex[group_id]);


    //put-fork
    pthread_mutex_lock(&printing_mutexes[station_id]);
    student->status = "REST";

    //informing group mate first
    int group_last = group_id*M;
    int group_first = group_last-M+1;
    for(int i=group_first; i<=group_last; i++)
    {
        int sti = i% NUM_PRINTING_STATIONS+1;
        if(sti==station_id)
            check(i);
    }
    //others
    for(int i=1; i<=N; i++)
    {
        int sti = i% NUM_PRINTING_STATIONS+1;
        if(sti==station_id && !(i>=group_first && i<=group_last))
        {
            check(i);
        }
    }
    pthread_mutex_unlock(&printing_mutexes[station_id]);


    //task 2 and task 3 writer part
    if(isLeader)
    {
        sem_wait(&student->sem);

        sleep(getRand()); //random sleep before group leader goes for binding

        str = "Group "+to_string(group_id)+" has arrived binding station at time "+to_string(getTime())+"\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);

        sem_wait(&binding_sem);

        str = "Group "+to_string(group_id)+" has started binding at time "+to_string(getTime())+"\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);


        sleep(x);


        str = "Group "+to_string(group_id)+" has finished binding at time "+to_string(getTime())+"\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);

        sem_post(&binding_sem);

        sleep(getRand()); //random sleep before group leader goes for submission

        str = "Group "+to_string(group_id)+" has arrived for submission at time "+to_string(getTime())+"\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);

        //writer
        pthread_mutex_lock(&write_mut);

        str = "Group "+to_string(group_id)+" has started submission at time "+to_string(getTime())+"\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);

        sleep(y);
        num_of_submission++;

        str = "Group "+to_string(group_id)+" has submitted the report at time "+to_string(getTime())+"\n\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);
        cout<<str;
        pthread_mutex_unlock(&write_mut);


    }

    return NULL;
}

void* library_staff_action(void* arg)
{
    int staff_id = *(int*)arg;
    while(true)
    {
        //reader
        sleep(getRand2()); //Random break of reading

        pthread_mutex_lock(&entry_book_mutex);
        rc++;
        if(rc==1)
            pthread_mutex_lock(&write_mut);
        pthread_mutex_unlock(&entry_book_mutex);

        string str = "Staff "+to_string(staff_id+1)+" has started reading the entry book at time "+to_string(getTime())+". No. of submission = "+to_string(num_of_submission)+"\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);

        sleep(y);

        str = "Staff "+to_string(staff_id+1)+" has finished reading the entry book at time "+to_string(getTime())+". No. of submission = "+to_string(num_of_submission)+"\n";
        pthread_mutex_lock(&file_output_mutex);
        outFile<<str;
        pthread_mutex_unlock(&file_output_mutex);

        pthread_mutex_lock(&entry_book_mutex);
        rc--;
        if(rc==0)
            pthread_mutex_unlock(&write_mut);
        pthread_mutex_unlock(&entry_book_mutex);

        if(num_of_submission==(N/M))
            break;
    }
    return NULL;
}

int main()
{
    initialTime = chrono::steady_clock::now();
    inFile.open("input.txt");
    outFile.open("output.txt");
    inFile>>N>>M>>w>>x>>y;
    int num_of_group = (N/M);
    students_info = new Student[N+1];
    print_completed = new int[num_of_group+1];
    group_mutex = new pthread_mutex_t[num_of_group+1];

    pthread_t students[N+1];
    pthread_t library_staff[2]; // Two staff members for the library

    // Initialize semaphores
    for (int i = 0; i <= NUM_PRINTING_STATIONS; i++)
    {
        pthread_mutex_init(&printing_mutexes[i], NULL);
    }

    sem_init(&binding_sem, 0, NUM_BINDING_STATIONS);
    pthread_mutex_init(&entry_book_mutex, NULL);
    pthread_mutex_init(&file_output_mutex, NULL);

    for (int i = 0; i <= num_of_group; i++)
    {
        pthread_mutex_init(&group_mutex[i], NULL);
        print_completed[i]=0;
    }

    vector<int> studentID(N);
    for (int i = 1; i <=N; i++)
    {
        studentID[i-1] = i;
    }

    shuffle(studentID.begin(), studentID.end(), gen);


    for (int i = 0; i<N; i++)
    {
        int sid = studentID[i];

        //string str = "Student "+to_string(sid)+" left for print station\n";
        //cout<<str;

        students_info[sid].student_id = sid;
        students_info[sid].status = "REST";
        sem_init(&students_info[sid].sem, 0, 0);

        pthread_create(&students[sid], NULL, student_action, &students_info[sid]);
    }
    pthread_mutex_init(&write_mut, NULL);
    int staff_ids[2] = {0, 1};
    for (int i = 0; i < 2; i++)
    {
        pthread_create(&library_staff[i], NULL, library_staff_action, &staff_ids[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i <= N; i++)
    {
        pthread_join(students[i], NULL);
    }
    for (int i = 0; i < 2; i++)
    {
        pthread_join(library_staff[i], NULL);
    }
    // Cleanup
    for (int i = 0; i <= NUM_PRINTING_STATIONS; i++)
    {
        pthread_mutex_destroy(&printing_mutexes[i]);
    }
    sem_destroy(&binding_sem);
    for (int i = 0; i <=num_of_group; i++)
    {
        pthread_mutex_destroy(&group_mutex[i]);
    }
    pthread_mutex_destroy(&entry_book_mutex);
    pthread_mutex_destroy(&file_output_mutex);
    pthread_mutex_destroy(&write_mut);

    inFile.close();
    outFile.close();
}
