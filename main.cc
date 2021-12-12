/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

const int INSUFFICIENT_NUMBER_OF_PARAMETERS = 1;
const int INVALID_PARAMETER = 2;
const int SEM_INIT_ERROR  = 3;
struct Job;
void *producer (void *id);
void *consumer (void *id);
int depositItem(Job& job);
Job fetchItem();
struct Job{
	int id;
	int duration;
};

struct circularQueue{
	int front = 0;
	int tail = 0;
	int size;
	Job* jobs;
	void initializeQueue(int qSize){
		size = qSize;
		front = 0;
		tail = 0;
		jobs = new Job [size];
	}
	~circularQueue(){ delete[] jobs; }
};
struct ProducerParam{
	int semId;
	int jobNum;
	int prodId = 0;
	ProducerParam(){}
	ProducerParam(int semId, int jobNum,int prodId): semId(semId),jobNum(jobNum),prodId(prodId) {}
};
struct ConsumerParam{
	int semId;
	int conId = 0;
	ConsumerParam(){}
	ConsumerParam(int semId,int conId): semId(semId),conId(conId){}
};



circularQueue myQueue;

const int space = 0,item = 1,mutex = 2;

int main (int argc, char **argv)
{

  // TODO
  if(argc < 5){
  	cerr<<"Insufficient number of parameters"<<endl;
  	exit(INSUFFICIENT_NUMBER_OF_PARAMETERS);
  }
  int qSize = check_arg(argv[1]);
  if(qSize == -1) {
  	cerr<<"Queue size error"<<endl;
	exit(INVALID_PARAMETER);
  } 
  int jobNum = check_arg(argv[2]);
  if(jobNum == -1){
  	cerr<<"JobNum error"<<endl;
	exit(INVALID_PARAMETER);
  }
  int produceNum = check_arg(argv[3]);
  if(produceNum == -1){
  	cerr<<"producer job number error"<<endl;
	exit(INVALID_PARAMETER);
  } 
  int consumeNum = check_arg(argv[4]);
  if(consumeNum == -1){
  	cerr<<"consumer number error"<<endl;
	exit(INVALID_PARAMETER);
  } 
  //create sem
  int Id = sem_create(SEM_KEY,3); 
  if(Id == -1){
  	cerr<<"get semId error"<<endl;
	exit(SEM_INIT_ERROR);
  } 
 //init num = 3?
  if(sem_init(Id,space,qSize)==-1){
  	cerr<<"get space sem init error"<<endl;
	exit(SEM_INIT_ERROR);
  } 
  if(sem_init(Id,item,0) == -1){
  	cerr<<"get item sem init error"<<endl;
	exit(SEM_INIT_ERROR); //item
  }
  if(sem_init(Id,mutex,1) == -1) {
 	cerr<<"get mutex init error"<<endl;
    exit(SEM_INIT_ERROR);	
  }
  myQueue.initializeQueue(qSize);
  ProducerParam *producerParam = new ProducerParam [produceNum];

  ConsumerParam *consumerParam = new ConsumerParam [consumeNum];
  srand(time(NULL));
// create process
  pthread_t producerId[produceNum];
  pthread_t consumerId[consumeNum];
  for(int i = 0;i < produceNum; i++){
	ProducerParam tmp(Id,jobNum,i);
	producerParam[i] = tmp;
	pthread_create(&producerId[i],NULL,producer,(void *) &producerParam[i]);
  }
  for(int i = 0;i < consumeNum; i ++){
	ConsumerParam tmp(Id,i);
	consumerParam[i] = tmp;
  	pthread_create (&consumerId[i],NULL,consumer,(void *) &consumerParam[i]);
  }
  // wait

  for(int i = 0;i < produceNum; i++){
  	pthread_join(producerId[i],NULL);

  }
  for(int i = 0; i < consumeNum; i ++){
  	pthread_join (consumerId[i],NULL);
  }
/*
  pthread_t producerid;
  int parameter = 5;

  pthread_create (&producerid, NULL, producer, (void *) &parameter);

  pthread_join (producerid, NULL);
*/
  cout << "Producer and consumer finished" << endl;
  sem_close(Id);
  delete []producerParam;
  delete []consumerParam;

  return 0;
}

// produce rand between l and r
int produceRand(int l, int r){
	int res = l + rand()%(r-l + 1);
	return res;	
}

void *producer (void *parameter) 
{

  // TODO

  ProducerParam *param = (ProducerParam *) parameter;
  Job job;
  for(int i = 0;i < param->jobNum;i ++){
  	int duration = produceRand(1,10);
	job.duration = duration;
	sleep(produceRand(1,5));
	if(sem_timeout(param->semId,space,TIMELIMIT)) {
		cerr<<"Producer "<<param->prodId<<" timeout"<<endl;
		pthread_exit(0);
	}
	sem_wait(param->semId,mutex);
	// deposit
	 depositItem(job);
	cerr<<"Producer"<<"("<<param->prodId<<"): "<<"Job id "<<job.id<<" duration "<<duration<<endl;
	sem_signal(param->semId,mutex);
	sem_signal(param->semId,item);

		
  }
  cerr<<"Producer("<<param->prodId<<") "<<" has no more job to generate"<<endl;
 // sleep (5);

  //cout << "\nThat was a good sleep - thank you \n" << endl;

  pthread_exit(0);
}

void *consumer (void *parameter) 
{
    // TODO
  ConsumerParam *param = (ConsumerParam *) parameter;
  while(!sem_timeout(param->semId,item,TIMELIMIT)){
  	sem_wait(param->semId,mutex);
	Job tempJob = fetchItem();
	sem_signal(param->semId,mutex);
	sem_signal(param->semId,space);
	//consume item
	cerr<<"Consumer"<<"("<<param->conId<<")"<<": "<<"Job id "<<tempJob.id<<" executing sleep duration "<<tempJob.duration<<endl;
	sleep(tempJob.duration);
	cerr<<"Consumer "<<"("<<param->conId<<")"<<": "<<"Job id "<<tempJob.id<<" completed"<<endl;	
  }  
  cerr<<"consumer "<<"("<<param->conId<<"): "<<"No more jobs left"<<endl;

  pthread_exit (0);

}

int depositItem(Job& job){
	job.id = myQueue.tail;
	myQueue.jobs[myQueue.tail] = job;
	myQueue.tail = (myQueue.tail+1)%myQueue.size;
	return myQueue.tail;
}

Job fetchItem(){
	Job tmp = myQueue.jobs[myQueue.front];
	myQueue.front = (myQueue.front + 1)%myQueue.size;
	return tmp;
	
}



