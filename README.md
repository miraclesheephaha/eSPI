# eSPI  Enhanced Serial Peripheral Interface
eSPI (Enhanced Serial Peripheral Interface) 是電腦主機板上的「神經總線」，它主要負責連接 PCH (主控端/南橋) 與 EC/BMC (嵌入式控制器)，處理那些「電腦開機前」以及「系統運行中」的瑣碎管理任務。  
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
> * 定義：Flash 晶片實體連接在 Master (PCH) 上。
> * 運作：EC (Slave) 若需要讀取自己的韌體，必須透過 eSPI 的 Flash Access Channel 向 PCH 發出請求，由 PCH 代為讀取後再傳回 EC。
> * 優點：節省成本，整台機器只需要一顆 Flash 晶片
2. SAFS (Slave Attached Flash Sharing)
> * 定義：Flash 晶片實體連接在 Slave (EC) 上。
> * 運作：PCH 啟動時所需的身分辨識或初始化代碼，需透過 eSPI 向 EC 請求獲取。
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

## Signal Description
### 🔌 eSPI 介面接腳定義 (Pin Definition)
> eSPI 透過虛擬化通道，大幅縮減了 PCH 與 EC/BMC 之間的實體接腳需求。

| 訊號 | 類型 | 說明 |
| :--- | :--- | :--- |
| **CLK** | Input | 由 PCH 提供的同步時脈。 |
| **IO[0:3]** | Bi-dir | 雙向資料線。**IO1** 支援 x2/x4 模式提昇傳輸效率。 |
| **CS#** | Input | 啟動傳輸的選取訊號。 |
| **Alert#** | Output | (可選) 實體警告訊號，單一 Slave 下建議改用 **In-band** 模式以節省 GPIO。 |

### ESPI Clock(O)
eSPI clock output from the PCH to slave device
### ESPI Data Signal(IO)
英：ESPI Data Signal 1: **Bi-directional** pin used to **transfer data** between the PCH and eSPI slave device.  
中：這根針腳是 「雙向傳遞資料」 的通道，無論是 PCH 要傳資料給從屬設備（Slave），還是從屬設備要回傳資料給 PCH，都會用到這條線。  
1. **Bi-directional** (雙向)： 這代表 IO1 腳位並非固定的「輸入」或「輸出」。在傳輸的過程中，它會根據當下的 方向轉折（Turn-around Cycle） 切換角色：
> * Master 傳給 Slave： PCH 驅動訊號（Output），Slave 接收。
> * Slave 傳給 Master： Slave 驅動訊號（Output），PCH 接收。
2. **Transfer data** (傳輸資料)： eSPI 支援多種資料頻寬模式，IO1 的參與程度取決於目前使用的 Bus Mode：
> * Single I/O Mode (x1)： 只有 IO0 傳資料，IO1 主要是輔助角色或閒置。
> * Dual I/O Mode (x2)： 使用 IO0 和 IO1 同時傳輸（速度翻倍）。
> * Quad I/O Mode (x4)： 使用 IO0, IO1, IO2, IO3 四根線同時傳輸（速度最高）。
### ESPI Chip Select(O)
英：ESPI Chip Select 0: **Driving CS# signal low** to **select eSPI slave** for the transaction.  
1. **Driving CS# signal low** (拉低訊號)： 在閒置狀態（Idle）時，CS# 會維持在高電位（High）。當 PCH 要開始跟某個 Slave 講話時，會把這根針腳的電壓拉低到 0V (Low)。這就像是在點名：「嘿！就是你，請準備好接收資料。」
2. **Select eSPI slave** (選取從屬端)： 一個 PCH 可能連接多個 Slave（雖然常見是一個，但規範支援多個）。每個 Slave 會有自己獨立的 CS# 線路。
> * CS0# 被拉低：選中第一個 Slave（通常是 EC）。
> * CS1# 被拉低：選中第二個 Slave（如果有）。
3. The transaction (傳輸事務/交易)： 代表一次完整的數據交換過程（從下指令到結束）。CS# 必須在整個傳輸過程中全程維持低電位，直到最後一個位元傳完後才會拉回高電位（De-assert）。
### ESPI Alert(I)
英：If only a single Slave is connected, the eSPI Compatibility Specification requires that the Slave must operate with **in-band Alert# signaling** in order to **free up the GPIO pin** required for the **discrete Alert# pin**  
中：當系統中只有一個 eSPI Slave（例如只有一顆 EC）時，不准使用實體線路來傳送 Alert# 訊號，必須改用「虛擬（內建）」的訊號傳輸。
1. **Discrete Alert# pin** (實體接腳)： 原本 eSPI 除了主要的資料線和時脈線外，還有一根獨立的 ALRT# 接腳，專門讓 Slave（從屬端）用來主動叫醒 Master（主控端）或是請求注意。
2. **In-band Alert# signaling** (頻內訊號)： 這是 eSPI 的黑科技。它不需要那根實體的 ALRT# 接腳，而是直接在原本的 Data Line (IO0) 上，透過特定的電氣特性（例如在時脈停止時拉低資料線）來傳遞警告訊息。
3. **Free up the GPIO pin** (釋放 GPIO 接腳)： 既然訊號已經走資料線了，原本預留給 ALRT# 的那根實體接腳就可以省下來，讓 PCH 或 EC 去做別的事（當成一般的 GPIO 使用）。
#### eSPI 傳輸的標準時序
當 PCH 要發起一個動作時，硬體上的動作順序如下：
1. 選取： PCH 將 CS# 拉低。
2. 同步： PCH 開始送出 CLK (Clock)。
3. 傳輸： PCH 與 Slave 透過 IO[0:3] 交換資料。
4. 結束： PCH 停止 CLK，並將 CS# 拉回高電位

(An eSPI transaction consists of a Command phase driven by the master, a TAR phase, and a Response phase driven by the slave)
<img width="767" height="215" alt="image" src="https://github.com/user-attachments/assets/1cbf6479-7f8d-4772-9e4a-bfbfe3ad098e" />

| 動作方向 | 驅動資料 (Drive) | 取樣資料 (Sample) |
|:---|:---|:---|
|**PCH 傳給 Slave**|下降沿 (Falling Edge)|上升沿 (Rising Edge)|
|**Slave 傳給 PCH**|下降沿 (Falling Edge)|下降沿 (Falling Edge)|

**The serial clock must be low at the assertion edge of the CS# while ESPI_RESET# has been de-asserte.** The first data is driven out from the PCH while the serial clock is still low and sampled on the rising edge of the clock by the slave. **Subsequent data is driven on the falling edge of the clock from the PCH and sampled on the rising edge of the clock by the slave.** Data from the slave is driven out on the falling edge of the clock and is sampled on a falling edge of the clock by the PCH.  
### 📦 資料傳輸單位 
eSPI 協議強制要求數據傳輸的對齊：  
eStandard Alignment  
* 所有 Transaction 必須為 **8-bit (1 Byte)** 的倍數。(All transactions on eSPI are in eight bit multiples.)
* 系統不支持非字節對齊 (Non-byte aligned) 的傳輸。  

Bus Mode vs Clocks per Byte
* **Single Mode (x1):** 8 clocks per byte.
* **Dual Mode (x2):** 4 clocks per byte.
* **Quad Mode (x4):** 2 clocks per byte.
#### WAIT State from eSPI Slave
英：There are situations when the slave cannot predict the length of the command packet from the master (PCH). For non-posted transactions, the slave is allowed to respond with a limited number of WAIT states. A WAIT state is a one byte response code. They must be the first set of response bytes from the slave after the TAR cycles.  
中：在某些情況下，從屬端 (Slave) 無法預知來自對端 (Master/PCH) 指令封包的長度。對於『非即發性交易 (Non-posted transactions)』，從屬端被允許回傳有限數量的 WAIT 狀態 (WAIT states) 作為回應。「WAIT 狀態是一種 1 位元組 (1 Byte) 的回應碼。在周轉週期 (TAR cycles) 結束後，它們必須是從屬端發出的第一組回應位元組。」

#### In-Band Link Reset
In case the eSPI link may end up in an undefined state (for example when a CRC error is received from the slave in a response to a Set_Configuration command), the PCH issues an In-Band Reset command which resets the eSPI link to the default configuration. This allows the controller to re-initialize the link and reconfigure the slave.  
當 eSPI 連結陷入 未定義狀態 時（例如：當 PCH 發送 Set_Configuration 指令後，卻收到來自 Slave 的 CRC 錯誤回應），PCH 會發出一個 『頻內重置 (In-Band Reset)』 指令。此指令會將 eSPI 連結重置回 預設配置 狀態，讓控制器能夠重新初始化連結並對 Slave 進行重新設定。  

#### eSPI Channel and Supported Transactions
An eSPI channel provides a means to allow multiple independent flows of traffic to share the same physical bus. Refer to the eSPI specification for more detail.  
Each of the channels has its dedicated resources such as queue and flow control. There is no ordering requirement between traffic from different channels.  
The number of types of channels supported by a particular eSPI slave is discovered through the GET_CONFIGURATION command issued by the PCH to the eSPI slave 
during initialization.  
eSPI 通道提供了一種機制，允許數個獨立的流量在同一組實體匯流排上進行共享。更詳細的資訊請參考 eSPI 規範。  
每一個通道都擁有其專屬的資源，例如隊列（Queue）與流量控制（Flow Control）。不同通道之間的流量並沒有先後順序的要求。  
特定 eSPI 從屬端（Slave）所支援的通道類型與數量，是在初始化過程中，由 PCH 對其發送 GET_CONFIGURATION 指令後發掘而得。
|通道|功能|例子|
|:---|:---|:---|
|Channel 0|周邊存取 (Peripheral)|傳統 I/O (80 Port), Memory 存取|
|Channel 1|虛擬線路 (Virtual Wire)|插拔偵測、中斷訊號、電源訊號 (PWR_GD)|
|Channel 2|頻外通訊 (OOB)|BMC 管理數據、溫度監控|
|Channel 3|Flash 存取 (Flash)|PCH 與 EC 共享 BIOS ROM|

<img width="537" height="144" alt="image" src="https://github.com/user-attachments/assets/33cc346c-09ee-495b-a6bc-42c7fa87ec49" />

<img width="694" height="706" alt="image" src="https://github.com/user-attachments/assets/8df89336-0ec2-4182-a343-c1cdf657f1d1" />

1. Peripheral Channel (Channel 0) Overview
eSPI 的 Channel 0 (Peripheral Channel，周邊通道) 的具體功能。它是為了讓 eSPI 能夠 100% 兼容舊有的 LPC 匯流排 而設計的。
The Peripheral channel performs the following Functions:  
* Target for PCI Device D31:F0: The eSPI controller duplicates the legacy LPC PCI Configuration space registers. These registers are mostly accessed via the BIOS, though some are accessed via the OS as well.
> 作為 PCI 裝置 D31:F0 的目標： eSPI 控制器複製了傳統 LPC 的 PCI 配置空間暫存器（PCI Configuration Space Registers）。這些暫存器大部分由 BIOS 存取，部分也會由作業系統（OS）存取。(ACPI，UART等都是通过Peripheral Channel传输数据(0x0080,0x0060/0x0064))
* Tunnel all Host to eSPI slave (EC/SIO) debug device accesses: these are the accesses that used to go over the LPC bus. These include various programmable and fixed I/O ranges as well as programmable Memory ranges. The programmable ranges and their enables reside in the PCI Configuration space.
> 隧道化（Tunneling）所有主機至 eSPI 從屬端（如 EC/SIO）的偵錯裝置存取： 這些是過去透過 LPC 匯流排進行的存取動作，包括各種可程式化與固定範圍的 I/O 位址，以及可程式化的記憶體範圍。這些範圍的設定與啟用開關位於 PCI 配置空間中。
* Tunnel all accesses from the eSPI slave to the Host. These include Memory Reads and Writes.
> 隧道化所有從 eSPI 從屬端至主機的存取： 這包括由從屬端發起的記憶體讀取（Memory Reads）與寫入（Memory Writes）。
2. Vertual Wire Channel (Channel 1) Overview
eSPI 的 Channel 1 (Virtual Wire Channel，虛擬線路通道) 的運作機制。這是 eSPI 能夠大幅減少主機板走線（Pin Count）的核心原因。
* Sideband and GPIO Pins: System events and other dedicated signals between the PCH and eSPI slave. These signals are tunneled between the two components over eSPI.
> 側頻與 GPIO 接腳（Sideband and GPIO Pins）： 處理 PCH 與 eSPI 從屬端（Slave）之間的系統事件及其他專屬訊號。這些訊號透過 eSPI 隧道化（Tunneled）技術在兩個組件之間傳輸。
* Serial IRQ Interrupts: Interrupts are tunneled from the eSPI slave to the PCH. Both edge and triggered interrupts are supported.
> 串列 IRQ 中斷（Serial IRQ Interrupts）： 中斷訊號由 eSPI 從屬端隧道化傳輸至 PCH。支援「邊緣觸發（Edge）」與「準位觸發（Level-triggered）」兩種中斷模式。
🔍 深度解析：虛擬化帶來的變革
對於 BIOS 與硬體工程師來說，這個通道的關鍵在於**「封包化」**。  
1. 什麼是「隧道化（Tunneling）」？
在傳統的 LPC 時代，如果你需要傳送一個「電源鍵按下」或「電池充滿」的訊號，你必須在 PCH 和 EC 之間拉一根實體的電線（Sideband Pin）。  
* eSPI 的作法： 它把這些電訊號的「高/低電位」狀態轉化成一個數位封包（Message），直接走 eSPI 的資料線。  
* 結果： 硬體上不需要實體線路，但邏輯上 PCH 覺得它好像還是接到了一根真實的接腳。  
2. 常見的虛擬訊號 (Virtual Wire Groups)
BIOS 工程師在設定時，通常會遇到這類訊號被打包成不同的 Index：  
* 系統狀態： 如 SLP_S3# (睡眠狀態)、PLTRST# (平台重置)。  
* 電源事件： 如 SMI#、SCI#、PME#。  
3. 串列 IRQ (Interrupts) 的整合
* 背景： 以前的設備（如鍵盤、滑鼠）發出中斷請求時，需要透過 SERIRQ 協定或獨立中斷線。  
* eSPI 做法： 現在全部整合進 Channel 1。當 EC 需要告訴 CPU「有人動了滑鼠」時，它會發送一個虛擬線路封包，PCH 收到後會解析出這是哪個 IRQ（例如 IRQ1 或 IRQ12），再轉交給處理器。  
* 觸發模式：  
> * Edge (邊緣觸發)： 訊號從低變高或從高變低時觸發。  
> * Level (準位觸發)： 只要訊號維持在某個電位就持續觸發。  








