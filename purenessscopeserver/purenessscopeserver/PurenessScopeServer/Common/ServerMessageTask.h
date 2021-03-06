#ifndef _SERVERMESSAGETASK_H
#define _SERVERMESSAGETASK_H

#include "define.h"
#include "ace/Task.h"
#include "ace/Synch.h"
#include "ace/Malloc_T.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"
#include "ace/Date_Time.h"

#include "IClientManager.h"
#include "MessageBlockManager.h"

//处理服务器间接收数据包过程代码
//如果服务器间线程处理挂起了，会尝试重启服务
//add by freeeyes

#define MAX_SERVER_MESSAGE_QUEUE 1000    //允许最大队列长度
#define MAX_DISPOSE_TIMEOUT      30      //允许最大等待处理时间  

//服务器间通讯的数据结构（接收包）
struct _Server_Message_Info
{
	IClientMessage*    m_pClientMessage;
	uint16             m_u2CommandID;
	ACE_Message_Block* m_pRecvFinish;
	_ClientIPInfo      m_objServerIPInfo;

	_Server_Message_Info()
	{
		m_u2CommandID    = 0;
		m_pClientMessage = NULL;
		m_pRecvFinish    = NULL;
	}
};

//服务器间数据包消息队列处理过程
class CServerMessageTask : public ACE_Task<ACE_MT_SYNCH>
{
public:
	CServerMessageTask();
	~CServerMessageTask();

	virtual int open(void* args = 0);
	virtual int svc (void);

	virtual int handle_signal (int signum,
		siginfo_t *  = 0,
		ucontext_t * = 0);

	bool Start();
	int  Close();
	bool IsRun();

	uint32 GetThreadID();

	bool PutMessage(_Server_Message_Info* pMessage);

	bool CheckServerMessageThread(ACE_Time_Value tvNow);

	void AddClientMessage(IClientMessage* pClientMessage);

	void DelClientMessage(IClientMessage* pClientMessage);

private:
	bool CheckValidClientMessage(IClientMessage* pClientMessage);
	bool ProcessMessage(_Server_Message_Info* pMessage, uint32 u4ThreadID);

private:
	uint32               m_u4ThreadID;  //当前线程ID
	bool                 m_blRun;       //当前线程是否运行
	uint32               m_u4MaxQueue;  //在队列中的数据最多个数
	EM_Server_Recv_State m_emState;     //处理状态
	ACE_Time_Value       m_tvDispose;   //接收数据包处理时间

	//记录当前有效的IClientMessage，因为是异步的关系。
	//这里必须保证回调的时候IClientMessage是合法的。
	typedef vector<IClientMessage*> vecValidIClientMessage;
	vecValidIClientMessage m_vecValidIClientMessage;
};

class CServerMessageManager
{
public:
	CServerMessageManager();
	~CServerMessageManager();

	void Init();

	bool Start();
	int  Close();
	bool PutMessage(_Server_Message_Info* pMessage);
	bool CheckServerMessageThread(ACE_Time_Value tvNow);

	void AddClientMessage(IClientMessage* pClientMessage);
	void DelClientMessage(IClientMessage* pClientMessage);

private:
	CServerMessageTask*         m_pServerMessageTask;
	ACE_Recursive_Thread_Mutex  m_ThreadWritrLock; 
};


typedef ACE_Singleton<CServerMessageManager, ACE_Recursive_Thread_Mutex> App_ServerMessageTask;
#endif
