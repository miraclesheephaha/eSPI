//UEFI基礎型別定義(EFI_HANDLE, EFI_STATUS, UINTN)
#include <Uefi.h>
/*
 *UEFI標準函式庫
 *它對SystemTable進行封裝。
 *原本要寫gST->ConOut->OutputString(...)才能印字，有了此庫
 *只需填寫Print(L"Hello")
*/
#include <Library/UefiLib.h>
/*
 * UefiBootServicesTableLib:
 * 主要是為了使用全域變數gBS，透過它可以呼叫：
 * gBS->Stall()：微秒延遲
 * gBS->AllocatePool()：分配記憶體
 * gBS->LocateProtocol()：尋找硬體協定(UART/DISK)
*/
#include <Library/UefiBootServicesTableLib.h>
/*
 *硬體I/O操作函式庫
 *與硬體溝通的橋樑，負責將C語言轉換成CPU的IN/OUT指令
 *對指定的I/O Port或Memory Mapped I/O(MMIO)進存取
 *(IoWrite8,IoRead8)
*/
#include <Library/IoLib.h>

#define IndexPort 0x2E
#define DataPort 0x2F

/*
 *是UEFI定義的一種整數回傳類型，用來告訴呼叫者函式執行的結果
 *在64位元系統中，通常是UINTN(即unsigned long long)
 *0(EFI_SUCCESS)
 為何不直接int 因為不同平台的int長度可能不同
 EFI_STATUS確保了在UEFI環境下，錯誤代碼的格式是統一且跨平台的
*/
EFI_STATUS

//確保跨編譯器的函式呼叫規範一致性
EFIAPI
init_serial(port){
    IoWrite8(port+1,0x00);
    IoWrite8(port+3,0x80);
    IoWrite8(port+0,0x01);
    IoWrite8(port+1,0x00);
    IoWrite8(port+3,0x03);
    IoWrite8(port+2,0xC7);
    IoWrite8(port+4,0x0B);
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UefiMain(IN EFI_HANDLE ImageHandle,IN EFI_SYSTEM_TABLE *SystemTable){
    CHAR8 Test[]="UART"
    
    IoWrite8(IndexPort,0xA5);
    IoWrite8(IndexPort,0xA5);

    IoWrite8(IndexPort,0x07);
    IoWrite8(DataPort,0x02);

    IoWrite8(IndexPort,0x30);
    IoWrite8(DataPort,0x01);

    init_serial(0x3F8);
    for(int i=0;i<sizeof(Test);i++){
        IoWrite8(0X3F8,Test[i]);
    }

    IoWrite8(0x80,IoRead8(0x3F8));
    gBS->Stall(2000000);

    IoWrite8(IndexPort,0xAA);
    return EFI_SUCCESS;
}