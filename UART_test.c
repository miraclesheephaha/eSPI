#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/ShellLib.h>

#define IndexPort 0x2E
#define DataPort 0x2F

EFI_STATUS
EFIAPI
init_serial(UINTN Port){
    IoWrite8(Port+1,0x00);//Disable all interrupts
    IoWrite8(Port+3,0x80);//LCR(Line Control Register) Enable DLAB(set baud rate divisor)
    IoWrite8(Port+0,0x01);//(low byte)Set divisor to 1(1843200/(baud rate*16))=1, baud rate=115200
    IoWrite8(Port+1,0x00);//(high byte) if 0x01(high)+01(low)=0x0101(257), 1843200/(257*16)=448
    IoWrite8(Port+3,0x03);//LCR 8 bits, no parity, one stop bit
    IoWrite8(Port+2,0xC7);//Enable FIFO
    IoWrite8(Port+4,0x0B);//Enable IRQ

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable){
    CHAR8 Test[]="UART1";

    //password
    IoWrite8(IndexPort,0XA5);
    IoWrite8(IndexPort,0XA5);

    //select logical devices2(UART1)
    IoWrite8(IndexPort,0x07);
    IoWrite8(DataPort,0x02);

    //Enable UART1
    IoWrite8(IndexPort,0x30);
    IoWrite8(DataPort,0x01);

    init_serial(0x3F8);
    for(int i=0;i<sizeof(Test);i++){
        IoWrite8(0x3F8,Test[i]);
    }

    gBS->Stall(2000000);
    IoWrite8(0x80,0xCC);

    gBS->Stall(2000000);
    IoWrite8(0X80,IoRead8(0x3F8));

    gBS->Stall(2000000);
    IoWrite8(0x80,0xAA);

    gBS->Stall(2000000);
    IoWrite8(0X80,IoRead8(0x3F9));

    gBS->Stall(2000000);
    IoWrite8(0x80,0xCC);
    
    gBS->Stall(2000000);
    IoWrite8(0X80,IoRead8(0x3FA));

    gBS->Stall (2000000);
    IoWrite8(0x80, 0xAA);

    gBS->Stall (2000000);
    IoWrite8(0x80, IoRead8(0x3FC));

    gBS->Stall(2000000);
    IoWrite8(0x80,0xCC);

    //Disable by writing 0xAA once
    IoWrite8(IndexPort,0xAA);
    return EFI_SUCCESS;

}
