#include "xil_exception.h"
#include "xil_cache.h"
#include "xparameters.h"
#include <iostream>
#include "xtmrctr.h"
using namespace std;

int main()
{
	u32* cdma_ptr = (u32*) XPAR_AXI_CDMA_0_BASEADDR;
	u32* src_ptr = (u32*) XPAR_PS7_DDR_0_S_AXI_HP0_BASEADDR;
	u32* dst_ptr = (u32*) XPAR_PS7_DDR_0_S_AXI_HP2_BASEADDR;
	XTmrCtr TimerPtr;

	// Init contents of arrays
	for(int i =0; i <= 16; i++)
	{
    	*(src_ptr + i) = i;
	}

	for(int i =0; i <= 16; i++)
	{
    	*(dst_ptr + i) = -i;
	}

	// init Timer

	int xStatus = XTmrCtr_Initialize(&TimerPtr, XPAR_AXI_TIMER_0_DEVICE_ID);
	if(xStatus != XST_SUCCESS)
	{
   	cout <<  "TIMER INIT FAILED" << endl;
   	return 1;
	}

	// Timer stuff
	XTmrCtr_SetResetValue(&TimerPtr, 0, 0);
	XTmrCtr_SetOptions(&TimerPtr, XPAR_AXI_TIMER_0_DEVICE_ID, XTC_CAPTURE_MODE_OPTION);


	//reset
	*(cdma_ptr) = 0x00000004;

	//configure
	*(cdma_ptr) = 0x00000020;

	//load address of src_array
	*(cdma_ptr + 6) = 0x20000000;

	//load address of dst_array
	*(cdma_ptr + 8) = 0x30000000;

	Xil_DCacheFlush();

	//bytes to transfer
	*(cdma_ptr + 10) = 0x64;

	char c;
	cout << "Press a key" << endl;
	cin >> c;

    //start timer
    XTmrCtr_Start(&TimerPtr, 0);

    int idle_bit = *(cdma_ptr + 1) & 2;
    while(idle_bit == 0)
    {
   	 idle_bit = *(cdma_ptr + 1) & 2; //get idle bit
   	 if(idle_bit == 2) break;
    }

//    for(int i=0;i<=16;i++){
//   	 *(dst_ptr + i)=*(src_ptr + i);
//    }

    //end tranfer
    XTmrCtr_Stop(&TimerPtr, 0);
    unsigned int cnt = XTmrCtr_GetValue(&TimerPtr, 0);


	cout<< "Results After"<<endl;
	//output the results
	cout<<"src = || ";
	for(int i=0;i<=16;i++){
   	 cout << *(src_ptr + i) << " || ";
	}
	cout<<endl<<"dst = || ";;
	for(int i=0;i<=16;i++){
   	 cout << *(dst_ptr + i) << " || ";
	}
	cout<<endl;

	cout <<  cnt << " clk cycles to transfer" <<  endl;
	cout << "End of Application" << endl;
	return 0;
}
