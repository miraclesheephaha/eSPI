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
### Flash Channel Controller (MAFCC vs. SAFCC)
這是硬體內部的邏輯控制單元，負責處理上述 Sharing 模式的通訊協定：
* MAFCC (Master Attached Flash Channel Controller)：位於 PCH 端。它負責處理來自 Slave 的「遠端 Flash 存取要求」。它會判斷權限、位址映射，並控制實體 SPI 介面去讀取 Flash。
* SAFCC (Slave Attached Flash Channel Controller)：位於 EC 端。它負責將 EC 內部的存取請求封裝成 eSPI 封包發送給 Master，或在 SAFS 模式下處理 Master 的存取請求。
### Out-of-Band (OOB) Channel
這是 eSPI 四大通道之一（Channel 3）。
* 用途：取代了傳統的 SMBus 介面。
* 功能：專門用於晶片組間的管理資訊傳輸。例如：Intel ME (Management Engine) 與 EC 之間的溝通。這條通道獨立於 BIOS/OS 的資料傳輸，即使系統在待命狀態下也能運作。
### Turn-around Cycle (TAR)
這是物理層（Physical Layer）的一個短暫時間區間，用於切換資料線的控制權。
* 機制：eSPI 是半雙工（Half-duplex）通訊，Master 與 Slave 共用資料線。
* 動作：當 Master 傳完指令後，會釋放資料線控制權，等待 Slave 接手回應。這段「無人驅動」或「切換驅動者」的空檔（通常是 2 個 Clock）就稱為 Turn-around Cycle。
* 重要性：TAR 確保了兩端不會同時對電路驅動，避免發生訊號碰撞（Bus Contention）。
### 💻 LPC vs eSPI 架構比較
| 功能項目 | 傳統 LPC 做法 | eSPI 做法 |
| :--- | :--- | :--- |
| **Flash 存取** | 各自獨立，接腳多 | MAFS / SAFS 共享，接腳少 |
| **控制器邏輯** | 單純的狀態機 | MAFCC / SAFCC 複雜封包管理 |
| **頻外通訊** | 額外的 SMBus 線路 | Out-of-Band Channel 虛擬化 |
| **訊號切換** | 較長的延遲 | Turn-around Cycle 精確計時 |
