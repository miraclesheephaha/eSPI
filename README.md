# eSPI  Enhanced Serial Peripheral Interface
eSPI 透過通道化 (Channelization) 的設計，讓 PCH (Master) 與 EC (Slave) 能在極少的接腳下完成  
<img width="651" height="260" alt="image" src="https://github.com/user-attachments/assets/7800a0f1-3f56-4be6-be1d-2c038e3d085c"/>  
在 eSPI 體系中，EC 通常充當 Slave
1. 角色：負責筆電的電源管理、鍵盤控制、風扇轉速及電池狀態監控。
2. 進化：在 LPC 時代，EC 透過多組實體線路連接 PCH；
3. 在 eSPI 時代，EC 只需要一組 eSPI 介面即可處理所有溝通，包含取代原本的 SMBus 和中斷線。

