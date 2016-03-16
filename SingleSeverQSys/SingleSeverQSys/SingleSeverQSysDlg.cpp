
// SingleSeverQSysDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SingleSeverQSys.h"
#include "SingleSeverQSysDlg.h"
#include "afxdialogex.h"


extern "C"
{
#include "lcgrand.h"
};
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define Q_LIMIT 100  /* Limit on queue length. */
#define BUSY      1  /* Mnemonics for server's being busy */
#define IDLE      0  /* and idle. */

class SingleServerQ
{
public:
	int   next_event_type, num_custs_delayed, num_delays_required, num_events,
		num_in_q, server_status;
	float area_num_in_q, area_server_status, mean_interarrival, mean_service,
		sim_time, time_arrival[Q_LIMIT + 1], time_last_event, time_next_event[3],
		total_of_delays;
	int m_num_policies;

	int totalCust, maxQlen, cust1plus;
	float totalTime, maxPTime, maxDelay;

	void  initialize(void);
	void  timing(void);
	void  arrive(void);
	void  depart(void);
	void  report(void);
	void  update_time_avg_stats(void);
	float expon(float mean);

protected:
private:
}m_singleServerQ;

int m_random_seed = 1;
int click_run_time = 0;

char * cs2ca(CString str)
{
	char *ptr;
#ifdef _UNICODE
	LONG len;
	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	ptr = new char [len+1];
	memset(ptr,0,len + 1);
	WideCharToMultiByte(CP_ACP, 0, str, -1, ptr, len + 1, NULL, NULL);
#else
	ptr = new char [str.GetAllocLength()+1];
	sprintf(ptr,_T("%s"),str);
#endif
	return ptr;
}


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSingleSeverQSysDlg 对话框




CSingleSeverQSysDlg::CSingleSeverQSysDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSingleSeverQSysDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSingleSeverQSysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listctrl);
	DDX_Control(pDX, IDC_BUTTON2, calAvgBT);
}

BEGIN_MESSAGE_MAP(CSingleSeverQSysDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CSingleSeverQSysDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSingleSeverQSysDlg 消息处理程序

BOOL CSingleSeverQSysDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 4	&initial_inv_level, &num_months, &num_policies, &num_values_demand,
	// 	4	&mean_interdemand, &setup_cost, &incremental_cost, &holding_cost,
	// 	3	&shortage_cost, &minlag, &maxlag
	SetDlgItemText(S_mean_interarrival,"1");
	SetDlgItemText(S_mean_service,"0.5");
	SetDlgItemText(S_num_delays_required,"1000");
	SetDlgItemText(random_seed,"1");
	SetDlgItemText(time_end,"not available");
	CString t;
	t.Format(_T("run times: %d"), click_run_time);
	SetDlgItemText(IDC_run_times,t);

	m_listctrl.InsertColumn(0,"run time", 1, 50, -1);
	m_listctrl.InsertColumn(1,"Average delay in queue", 1, 100, -1);
	m_listctrl.InsertColumn(2,"Average number in queue", 1, 120, -1);
	m_listctrl.InsertColumn(3,"Server utilization", 1, 120, -1);
	m_listctrl.InsertColumn(4,"Time simulation ended", 1, 120, -1);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSingleSeverQSysDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSingleSeverQSysDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CSingleSeverQSysDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSingleSeverQSysDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	//至少计算一次后，可以计算平均；
	//calAvgBT.EnableWindow(TRUE);
	//m_listctrl.DeleteAllItems();
	//read params
	click_run_time++;
	CString tmp;
	char *str="";
	CString t;
	t.Format(_T("run times: %d"), click_run_time);
	SetDlgItemText(IDC_run_times,t);
	
	//first obtain the seed;
	m_random_seed = GetDlgItemInt(random_seed);
	m_singleServerQ.num_delays_required = GetDlgItemInt(S_num_delays_required);
	GetDlgItemText(S_mean_interarrival, tmp);
	str = cs2ca(tmp);
	m_singleServerQ.mean_interarrival = atof(str);
	GetDlgItemText(S_mean_service, tmp);
	str = cs2ca(tmp);
	m_singleServerQ.mean_service = atof(str);

	m_singleServerQ.num_events = 2;

	m_singleServerQ.initialize();

	while (m_singleServerQ.num_custs_delayed < m_singleServerQ.num_delays_required)     
	{
		/* Determine the next event. */

		m_singleServerQ.timing();

		/* Update time-average statistical accumulators. */

		m_singleServerQ.update_time_avg_stats();

		/* Invoke the appropriate event function. */

		switch (m_singleServerQ.next_event_type) 
		{
		case 1:
			m_singleServerQ.arrive();
			break;
		case 2:
			m_singleServerQ.depart();
			break;
		}
	}
	
	//m_singleServerQ.report();
	t.Format(_T("No. %.d"), click_run_time);
	m_listctrl.InsertItem(0, t);
	t.Format(_T("%.3f"), 
		m_singleServerQ.total_of_delays / 
		m_singleServerQ.num_custs_delayed);//number per minute
	m_listctrl.SetItemText(0, 1, t);
	t.Format(_T("%.3f"), 
		m_singleServerQ.area_num_in_q / 
		m_singleServerQ.sim_time);//avg total time(serve+delay)
	m_listctrl.SetItemText(0, 2, t);
	t.Format(_T("%.3f"), 
		m_singleServerQ.area_server_status /
		m_singleServerQ.sim_time);//max q length
	m_listctrl.SetItemText(0, 3, t);
	t.Format(_T("%.3f"), 
		m_singleServerQ.sim_time);//max q length
	m_listctrl.SetItemText(0, 4, t);


	//     fprintf(outfile, "\n\nAverage delay in queue%11.3f minutes\n\n",
	//             total_of_delays / num_custs_delayed);
	//     fprintf(outfile, "Average number in queue%10.3f\n\n",
	//             area_num_in_q / sim_time);
	//     fprintf(outfile, "Server utilization%15.3f\n\n",
	//             area_server_status / sim_time);
	//     fprintf(outfile, "Time simulation ended%12.3f minutes", sim_time);
}



void SingleServerQ::initialize(void)  /* Initialization function. */
{
    /* Initialize the simulation clock. */

    sim_time = 0.0;

    /* Initialize the state variables. */

    server_status   = IDLE;
    num_in_q        = 0;
    time_last_event = 0.0;

    /* Initialize the statistical counters. */

    num_custs_delayed  = 0;
    total_of_delays    = 0.0;
    area_num_in_q      = 0.0;
    area_server_status = 0.0;

    /* Initialize event list.  Since no customers are present, the departure
       (service completion) event is eliminated from consideration. */

    time_next_event[1] = sim_time + expon(mean_interarrival);
    time_next_event[2] = 1.0e+30;

	/*init params*/
	totalCust = 0;
	maxQlen = 0;
	cust1plus = 0;
	totalTime = 0;
	maxPTime = 0;
	maxDelay = 0;
}


void SingleServerQ::timing(void)  /* Timing function. */
{
    int   i;
    float min_time_next_event = 1.0e+29;

    next_event_type = 0;

    /* Determine the event type of the next event to occur. */

    for (i = 1; i <= num_events; ++i)
        if (time_next_event[i] < min_time_next_event)
        {   
            min_time_next_event = time_next_event[i];
            next_event_type     = i;
        }

    /* Check to see whether the event list is empty. */

    if (next_event_type == 0)
    {
        /* The event list is empty, so stop the simulation. */
// 
//         fprintf(outfile, "\nEvent list empty at time %f", sim_time);
//         exit(1);
    }

    /* The event list is not empty, so advance the simulation clock. */

    sim_time = min_time_next_event;
}


void SingleServerQ::arrive(void)  /* Arrival event function. */
{
    float delay;

	totalCust++;

    /* Schedule next arrival. */

    time_next_event[1] = sim_time + expon(mean_interarrival);

    /* Check to see whether server is busy. */

    if (server_status == BUSY)
    {
        /* Server is busy, so increment number of customers in queue. */

        ++num_in_q;

        /* Check to see whether an overflow condition exists. */

        if (num_in_q > Q_LIMIT)
        {
            /* The queue has overflowed, so stop the simulation. */
			exit(2);
//             fprintf(outfile, "\nOverflow of the array time_arrival at");
//             fprintf(outfile, " time %f", sim_time);
//             exit(2);
        }

		if (num_in_q > maxQlen)
		{
			maxQlen = num_in_q;
		}

        /* There is still room in the queue, so store the time of arrival of the
           arriving customer at the (new) end of time_arrival. */

        time_arrival[num_in_q] = sim_time;
    }

    else
    {
        /* Server is idle, so arriving customer has a delay of zero.  (The
           following two statements are for program clarity and do not affect
           the results of the simulation.) */

        delay            = 0.0;
        total_of_delays += delay;

        /* Increment the number of customers delayed, and make server busy. */

        ++num_custs_delayed;
        server_status = BUSY;

        /* Schedule a departure (service completion). */
		float tmptime = expon(mean_service);
        time_next_event[2] = sim_time + tmptime;
		totalTime += tmptime;
    }
}


void SingleServerQ::depart(void)  /* Departure event function. */
{
    int   i;
    float delay;

    /* Check to see whether the queue is empty. */

    if (num_in_q == 0)
    {
        /* The queue is empty so make the server idle and eliminate the
           departure (service completion) event from consideration. */

        server_status      = IDLE;
        time_next_event[2] = 1.0e+30;
    }

    else
    {
        float thisPersonTime = 0;
		/* The queue is nonempty, so decrement the number of customers in
           queue. */

        --num_in_q;

        /* Compute the delay of the customer who is beginning service and update
           the total delay accumulator. */

        delay            = sim_time - time_arrival[1];
        total_of_delays += delay;

		totalTime += delay;
		if (delay > maxDelay)
		{
			maxDelay = delay;
		}
		if (delay > 1.0)
		{
			cust1plus++;
		}
		thisPersonTime += delay;

        /* Increment the number of customers delayed, and schedule departure. */

        ++num_custs_delayed;

		float servetime = expon(mean_service);
		time_next_event[2] = sim_time + servetime;
		totalTime += servetime;

		thisPersonTime += servetime;

		if (thisPersonTime > maxPTime)
		{
			maxPTime = thisPersonTime;
		}

        /* Move each customer in queue (if any) up one place. */

        for (i = 1; i <= num_in_q; ++i)
            time_arrival[i] = time_arrival[i + 1];
    }
}


void SingleServerQ::report(void)  /* Report generator function. */
{
    /* Compute and write estimates of desired measures of performance. */

//     fprintf(outfile, "\n\nAverage delay in queue%11.3f minutes\n\n",
//             total_of_delays / num_custs_delayed);
//     fprintf(outfile, "Average number in queue%10.3f\n\n",
//             area_num_in_q / sim_time);
//     fprintf(outfile, "Server utilization%15.3f\n\n",
//             area_server_status / sim_time);
//     fprintf(outfile, "Time simulation ended%12.3f minutes", sim_time);
}


void SingleServerQ::update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_time - time_last_event;
    time_last_event       = sim_time;

    /* Update area under number-in-queue function. */

    area_num_in_q      += num_in_q * time_since_last_event;

    /* Update area under server-busy indicator function. */

    area_server_status += server_status * time_since_last_event;
}


float SingleServerQ::expon(float mean)  /* Exponential variate generation function. */
{
    /* Return an exponential random variate with mean "mean". */

    return -mean * log(lcgrand(m_random_seed));
}

