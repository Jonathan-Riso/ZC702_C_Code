#include <iostream>
using namespace std;


#include "xparameters.h"
#include "xil_types.h"

#include "xtmrctr.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xscugic.h"
#include <stdio.h>

XScuGic InterruptController;

static XScuGic_Config *GicConfig;

XTmrCtr TimerInstancePtr;

int test = 0;

void Timer_InterruptHandler(void)
{
     unsigned int* timer_ptr = (unsigned int* )XPAR_AXI_TIMER_0_BASEADDR;
    // Stop both timers
    *(timer_ptr) = 0x54;
    *(timer_ptr+4) = 0x54;
    // read their values to see which trigger interupt
    if(*(timer_ptr) == 0x154) cout<<"timer 0 triggered interupt"<<endl;
    else cout<< "timer 1 triggered interupt"<<endl;
    // resume after being prompted
    char input;
    cout << "Press any key to restart the timers" << endl;
    cin >> input ;
    cout << "You pressed "<<  input << endl;
    //resuming
    *(timer_ptr) = 0x0d4;
    *(timer_ptr+4) = 0x0d4;
 }

int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{
    /*
    * Connect the interrupt controller interrupt handler to the hardware
    * interrupt handling logic in the ARM processor.
    */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
    (Xil_ExceptionHandler) XScuGic_InterruptHandler,
    XScuGicInstancePtr);
    /*
    * Enable interrupts in the ARM
    */
    Xil_ExceptionEnable();
    return XST_SUCCESS;
}

int ScuGicInterrupt_Init(u16 DeviceId,XTmrCtr *TimerInstancePtr)
{
    int Status;
    /*
    * Initialize the interrupt controller driver
    */

    GicConfig = XScuGic_LookupConfig(DeviceId);
    if (NULL == GicConfig)
    {
   	 return XST_FAILURE;
    }
    Status = XScuGic_CfgInitialize(&InterruptController, GicConfig,
    GicConfig->CpuBaseAddress);

    if (Status != XST_SUCCESS)
    {
   	 return XST_FAILURE;
    }
    /*
    * Setup the Interrupt System
    * */
    Status = SetUpInterruptSystem(&InterruptController);
    if (Status != XST_SUCCESS)
    {
   	 return XST_FAILURE;
    }

    // I don't know what this does but it resolves interupts only occuring once
    XScuGic_CPUWriteReg(&InterruptController, XSCUGIC_EOI_OFFSET, XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR);

    /*
    * Connect a device driver handler that will be called when an
    * interrupt for the device occurs, the device driver handler performs
    * the specific interrupt processing for the device
    */
    Status = XScuGic_Connect(&InterruptController,
   						  XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR,
   						  (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
   						  (void *)TimerInstancePtr);

    if (Status != XST_SUCCESS)
    {
   	 return XST_FAILURE;
    }
    /*
    * Enable the interrupt for the device and then cause (simulate) an
    * interrupt so the handlers will be called
    */
    XScuGic_Enable(&InterruptController, XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR);

    return XST_SUCCESS;
}


int main()
{
	cout << "Application starts " << endl;
	int xStatus;


    	// AXI Timer Initialization
    	xStatus = XTmrCtr_Initialize(&TimerInstancePtr, XPAR_AXI_TIMER_0_DEVICE_ID);
    	if(XST_SUCCESS != xStatus)
    	{
            	cout << "TIMER INIT FAILED " << endl;
            	if(xStatus == XST_DEVICE_IS_STARTED)
            	{
                    	cout << "TIMER has already started" << endl;
                    	cout << "Please power cycle your board, and re-program the bitstream" << endl;
            	}
            	return 1;
    	}





    // Set Timer Handler
    XTmrCtr_SetHandler(&TimerInstancePtr, (XTmrCtr_Handler)Timer_InterruptHandler, &TimerInstancePtr);



  	// intialize time pointer with value from xparameters.h file

    unsigned int* timer_ptr = (unsigned int* )XPAR_AXI_TIMER_0_BASEADDR;

    // load the reset value
   	*(timer_ptr+ 1) = 0xffffff00;
   	*(timer_ptr+ 5) = 0xfffffff0;

    //set the timer options
   	*(timer_ptr)  = 0x0f4 ;
   	*(timer_ptr+4)  = 0x0f4 ;







    //SCUGIC interrupt controller Initialization
    xStatus=
    ScuGicInterrupt_Init(XPAR_PS7_SCUGIC_0_DEVICE_ID, &TimerInstancePtr);
    if(XST_SUCCESS != xStatus)
    {
   	 cout << " SCUGIC INIT FAILED " << endl;
   	 return 1;
    }

    //Init completed

    //Await confimation before timer start
    char input;
    cout << "Press any key to start the timer" << endl;
    cin >> input ;
    cout << "You pressed "<<  input << endl;
	cout << "Enabling the timer to start" << endl;

    	*(timer_ptr) = 0x0d4 ;   // deassert the load 5 to allow the timer to start counting
    	*(timer_ptr+4) = 0x0d4 ;

    while( 1){
   	 char input;
   		 cout << "Press any key to start the timer" << endl;
   		 cin >> input ;
   		 if (input == 'e') break;
    } //  // wait forever and let the timer generate periodic interrupts

	return 0;
}
