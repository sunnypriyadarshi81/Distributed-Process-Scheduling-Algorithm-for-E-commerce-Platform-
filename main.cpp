#include<bits/stdc++.h>
using namespace std;

struct WorkerThread {
    int priority;
    int resources;
    int available;
    pthread_mutex_t mutex;
    bool isCompleted;
};

struct Request {
    int id;
    int type;
    int resources;
    chrono::high_resolution_clock::time_point at;
    chrono::high_resolution_clock::time_point st;
    chrono::high_resolution_clock::time_point ct;
    chrono::milliseconds wt;
    chrono::milliseconds tat;
};

struct Service {
    int type;
    int m;
    queue<Request*> serviceQueue;
    pthread_mutex_t mutex;
    vector<WorkerThread> threads;
    bool isCompleted;
};

vector<Service> services;

int rejectedRequests = 0;
int waitedCount = 0;
int blockedCount = 0;
pthread_mutex_t rejectedMutex;
pthread_mutex_t waitedMutex;
pthread_mutex_t blockedMutex;


bool is_service_queue_empty(int service_id){
    pthread_mutex_lock(&services[service_id].mutex);
    bool res = services[service_id].serviceQueue.empty();
    pthread_mutex_unlock(&services[service_id].mutex);
    return res;
}

void serviceQueuePush(int service_id, Request *req){
    pthread_mutex_lock(&services[service_id].mutex);
    services[service_id].serviceQueue.push(req);
    pthread_mutex_unlock(&services[service_id].mutex);
}

Request* serviceQueueFront(int service_id){
    pthread_mutex_lock(&services[service_id].mutex);
    Request *request = services[service_id].serviceQueue.front();
    services[service_id].serviceQueue.pop();
    pthread_mutex_unlock(&services[service_id].mutex);
    return request;
}

bool is_execution_queue_empty(int service_id, int thread_id, queue<Request*> &execution_queue){
    pthread_mutex_lock(&services[service_id].threads[thread_id].mutex);
    bool ret = execution_queue.empty();
    pthread_mutex_unlock(&services[service_id].threads[thread_id].mutex);
    return ret;
}

Request* executionQueueFront(int service_id, int thread_id, queue<Request*> &execution_queue){
    pthread_mutex_lock(&services[service_id].threads[thread_id].mutex);
    Request *req = execution_queue.front();
    execution_queue.pop();
    pthread_mutex_unlock(&services[service_id].threads[thread_id].mutex);
    return req;
}

void execute_request(int service_id, int thread_id, Request *req){
    req->st = chrono::high_resolution_clock::now();         
    printf("-----Service: %d thread: %d request: %d started.-----\n", service_id, thread_id, req->id);
    sleep(5);
    printf("-----Service: %d thread: %d request: %d completed.-----\n", service_id, thread_id, req->id);
    req->ct = chrono::high_resolution_clock::now();   
    pthread_mutex_lock(&services[service_id].threads[thread_id].mutex);
    services[service_id].threads[thread_id].available += req->resources; 
    pthread_mutex_unlock(&services[service_id].threads[thread_id].mutex);
    req->tat = chrono::duration_cast<chrono::milliseconds>(req->ct - req->at);          // TAT = CT - AT
    req->wt = chrono::duration_cast<chrono::milliseconds>(req->st - req->at);           // WT = TAT - BT
}

void worker_thread_function(int service_id, int thread_id, queue<Request*> &execution_queue){
    vector<thread> worker_thread_instances;
    while(true){
        if(!is_execution_queue_empty(service_id, thread_id, execution_queue)){
            Request *req = executionQueueFront(service_id, thread_id, execution_queue);
            worker_thread_instances.emplace_back(execute_request, service_id, thread_id, req);
        } else if (services[service_id].threads[thread_id].isCompleted) {
            break;
        }
        usleep(10);
    }

    for(thread& instance: worker_thread_instances){
        instance.join();
    }
}

bool comparator(WorkerThread a, WorkerThread b){
    return a.priority < b.priority;
}

void service_function(int service_id){
    int nt = services[service_id].m;
    thread worker_threads[nt];    
    queue<Request*> execution_queue[nt];

    sort(services[service_id].threads.begin(), services[service_id].threads.end(), comparator);
    
    for(int j=0; j<nt; j++){
        pthread_mutex_init(&services[service_id].threads[j].mutex, NULL);
        worker_threads[j] = thread(worker_thread_function, service_id, j, ref(execution_queue[j])); 
    }

    while(true){
        if(!is_service_queue_empty(service_id)){
            Request *req = serviceQueueFront(service_id);
            
            int lessResources = 0;
            bool allOccupied = true, assigned = false;

            for(int thread_id=0; thread_id<nt; thread_id++){
                pthread_mutex_lock(&services[service_id].threads[thread_id].mutex);
                if(req->resources <= services[service_id].threads[thread_id].available){  
                    services[service_id].threads[thread_id].available -= req->resources;  
                    printf("Service: %d request resource: %d assigned to thread: %d available: %d\n", service_id, req->resources, thread_id, services[service_id].threads[thread_id].available);
                    execution_queue[thread_id].push(req);
                    pthread_mutex_unlock(&services[service_id].threads[thread_id].mutex);
                    allOccupied = false;
                    assigned = true;
                    break;
                } else if (req->resources > services[service_id].threads[thread_id].resources){   
                    lessResources++;
                }
                if(services[service_id].threads[thread_id].resources == services[service_id].threads[thread_id].available){  
                    allOccupied = false;
                }
                pthread_mutex_unlock(&services[service_id].threads[thread_id].mutex);
            }

            if(lessResources == nt){    
                printf("Service: %d request id: %d with resources: %d is rejected.\n", service_id, req->id, req->resources);
                pthread_mutex_lock(&rejectedMutex);
                rejectedRequests++;
                pthread_mutex_unlock(&rejectedMutex);

            } else if (!assigned) {     
                serviceQueuePush(service_id, req);
                if(allOccupied){   
                    printf("Service: %d request id: %d with resources: %d is blocked.\n", service_id, req->id, req->resources);
                    pthread_mutex_lock(&blockedMutex);
                    blockedCount++;
                    pthread_mutex_unlock(&blockedMutex);
                } else {  
                    printf("Service: %d request id: %d with resources: %d is in waiting queue.\n", service_id, req->id, req->resources);
                    pthread_mutex_lock(&waitedMutex);
                    waitedCount++;
                    pthread_mutex_unlock(&waitedMutex);
                }
            }
        } else if(services[service_id].isCompleted){
            break;
        }
        usleep(100);
    }

    for (int j = 0; j<nt; j++) {
        worker_threads[j].join();
        pthread_mutex_destroy(&services[service_id].threads[j].mutex);
    }
}

void input(int n, int m){

}

int main(){
    srand(time(NULL));
    int n, m;
    printf("Enter the number of services: ");
    scanf("%d", &n);
    printf("Enter the number of threads per process: ");
    scanf("%d", &m);
    services.resize(n);
    thread service_threads[n];
    
    for(int i=0; i<n; i++){
        services[i].type = i;
        services[i].m = m;
        services[i].isCompleted = false;
        pthread_mutex_init(&services[i].mutex, NULL);
    }

    // Initial setup for services and worker threads -> Sare threads of each service ko priority and resources allocate kr diye.
    for(int i=0; i<n; i++){
        for(int thread_id=0; thread_id<m; thread_id++){
            int priority, resources;
            printf("Enter priority and resources for Service %d and thread %d\n", i, thread_id);
            scanf("%d %d", &priority, &resources);
            WorkerThread thread;
            thread.priority = priority;
            thread.resources = resources;
            thread.available = resources;
            thread.isCompleted = false;
            services[i].threads.push_back(thread);
        }
    }

    int number_requests;
    printf("Enter number of requests\n");
    scanf("%d", &number_requests);
    printf("Enter request type and resources.\n");

    for(int i=0; i<n; i++){
        service_threads[i] = thread(service_function, i);
    }

    int id = 0;
    vector<Request*> requests;

    for(int i=0; i<number_requests; i++){
        int transactionType, resources;
        scanf("%d %d", &transactionType, &resources);
        if (transactionType < 0 || transactionType >= n) {
            printf("Invalid transaction type %d\n", transactionType);
            continue;
        }
        printf("id: %d transactionType: %d resources: %d Request arrived\n", id, transactionType, resources);
        Request *req = new Request;
        req->id = id;
        req->type = transactionType;
        req->resources = resources;
        req->at = chrono::high_resolution_clock::now();
        requests.push_back(req);
        serviceQueuePush(transactionType, req);
        id++;
    }

    sleep(2);


    for(int i=0; i<n; i++) {
        services[i].isCompleted = true;
        for(int j=0; j<m; j++) {
            services[i].threads[j].isCompleted = true;
        }
    }

    for(int i=0; i<n; i++) {
        service_threads[i].join();
        pthread_mutex_destroy(&services[i].mutex);
    }

    printf("Number of rejected requests: %d.\n", rejectedRequests);
    for(Request *req: requests){
        printf("id: %d Waiting time: %ldms Turnaround time: %ldms\n", req->id, req->wt.count(), req->tat.count());
    }

    chrono::milliseconds average_waiting_time(0);
    chrono::milliseconds average_turnaround_time(0);
    int count = 0;
    for(Request *req: requests){
        if(req->tat.count() != 0){
            average_waiting_time += req->wt;
            average_turnaround_time += req->tat;
            count += 1;
        }
    }
    
    average_waiting_time /= count;
    average_turnaround_time /= count;
    printf("Average Waiting Time: %ldms\n", average_waiting_time.count());
    printf("Average Turnaround Time: %ldms\n", average_turnaround_time.count());

    return 0;
}