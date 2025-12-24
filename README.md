# eSPI  Enhanced Serial Peripheral Interface
eSPI 透過通道化 (Channelization) 的設計，讓 PCH (Master) 與 EC (Slave) 能在極少的接腳下完成  
<img width="651" height="260" alt="image" src="https://github.com/user-attachments/assets/7800a0f1-3f56-4be6-be1d-2c038e3d085c"/>  
### EC
在 eSPI 體系中，EC 通常充當 Slave
* 角色：負責筆電的電源管理、鍵盤控制、風扇轉速及電池狀態監控。
* 進化：在 LPC 時代，EC 透過多組實體線路連接 PCH；
* 在 eSPI 時代，EC 只需要一組 eSPI 介面即可處理所有溝通，包含取代原本的 SMBus 和中斷線。
### Flash Sharing (MAFS vs. SAFS)
這是 eSPI 最具革命性的功能。過去 BIOS Flash 通常由 PCH 控制，EC Flash 由 EC 控制。現在雙方可以「共享」同一顆 Flash 晶片，分為兩種模式：
1. MAFS (Master Attached Flash Sharing)
* 定義：Flash 晶片實體連接在 Master (PCH) 上。
* 運作：EC (Slave) 若需要讀取自己的韌體，必須透過 eSPI 的 Flash Access Channel 向 PCH 發出請求，由 PCH 代為讀取後再傳回 EC。
* 優點：節省成本，整台機器只需要一顆 Flash 晶片
2. SAFS (Slave Attached Flash Sharing)
* 定義：Flash 晶片實體連接在 Slave (EC) 上。
* 運作：PCH 啟動時所需的身分辨識或初始化代碼，需透過 eSPI 向 EC 請求獲取。
