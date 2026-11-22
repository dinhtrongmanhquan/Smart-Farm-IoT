# Smart-Farm-Arduino

Dự án **Mô hình Trang trại Thông minh ứng dụng hệ thống nhúng** nhằm giám sát môi trường và tự động hóa các tác vụ chăm sóc trang trại, có khả năng vận hành tự động theo thời gian/cảm biến, điều khiển thủ công trực tiếp tại chỗ và cảnh báo sự cố kịp thời.

## Tổng quan
Hệ thống được thiết kế theo kiến trúc đa vi điều khiển phân tầng, bao gồm:
- **Tầng thiết bị & Chấp hành (Device & Actuator Layer):** 03 bo mạch **Arduino UNO R3** phân chia nhiệm vụ độc lập (`UNO 1`: RTC DS1302, cảm biến ánh sáng, động cơ bước 28BYJ-48 qua driver ULN2003, đèn LED, công tắc/nút nhấn; `UNO 2`: cảm biến mực nước, cảm biến khí gas MQ-2, module relay điều khiển máy bơm DC; `UNO 3`: cảm biến DHT11, màn hình LCD I2C 16x2 địa chỉ `0x27`, còi Buzzer 2500Hz).
- **Tầng truyền thông (Communication Layer):** Giao tiếp nội bộ giữa `UNO 2` và `UNO 3` qua **UART (`SoftwareSerial`)** tốc độ `9600 baud` bằng khung dữ liệu **CSV 7 trường**, kết hợp kiểm tra `timeout` (`<= 3000ms`) và tín hiệu trạng thái số từ `UNO 1` sang `UNO 3`.
- **Tầng giao diện & Cảnh báo (Interface & Alert Layer):** Màn hình **LCD I2C 16x2** luân phiên hiển thị 4 trang thông số/trạng thái, kết hợp **còi Buzzer** cảnh báo tại chỗ khi phát hiện rò rỉ khí gas hoặc nhiệt độ vượt ngưỡng an toàn.

## Poster Dự Án
<p align="center">
  <img src="Smart-Farm-IoT/poster/poster.pdf" width="1600">
</p>

## Chức năng chính
- Tự động bơm cấp nước theo thuật toán vùng trễ (Hysteresis: bật bơm khi `< 100`, giữ trạng thái trong `100–499`, tắt bơm khi `>= 500`)
- Tự động cho ăn theo chu kỳ/thời gian thực (RTC DS1302) điều khiển động cơ bước qua driver ULN2003
- Tự động bật/tắt đèn chiếu sáng theo cảm biến ánh sáng và trạng thái chế độ đèn
- Giám sát nhiệt độ, độ ẩm (DHT11), mực nước và nồng độ khí gas (MQ-2) trên LCD I2C 4 trang
- Cảnh báo âm thanh (Buzzer) khi khí gas ở mức báo động (`Gas DO = LOW`) hoặc nhiệt độ `> 40°C`
- Chuyển đổi linh hoạt giữa chế độ tự động và điều khiển thủ công bằng nút nhấn/công tắc vật lý

## Công nghệ sử dụng
- 3× Arduino UNO R3 (`ATmega328P`), RTC DS1302, DHT11, MQ-2, cảm biến mực nước, cảm biến ánh sáng
- Giao thức UART (`SoftwareSerial` – 9600 baud, CSV 7 trường), I2C (`0x27`)
- Động cơ bước `28BYJ-48`, driver `ULN2003`, module `Relay`, máy bơm DC, LCD I2C `16x2`, Buzzer
- Thư viện Arduino: `AccelStepper`, `Rtc by Makuna`, `LiquidCrystal_I2C`, `DHT sensor library`, `Adafruit Unified Sensor`

## Hướng phát triển
- Tích hợp module Wi-Fi (`ESP32` / `ESP8266`) để gửi dữ liệu lên Web/App Dashboard và điều khiển từ xa
- Bổ sung cảm biến đo mức thức ăn trong phễu và lưu trữ log dữ liệu dài hạn
- Hoàn thiện cơ chế bảo vệ nguồn, chống nhiễu tải cảm và cảnh báo đa kênh

## 🤝 Đóng góp
Dự án được phát triển bởi Nhóm 2 – Khoa Công nghệ Thông tin, Trường Đại học Đại Nam (GVHD: ThS. Nguyễn Văn Nhân):

| Họ và Tên                | Vai trò                                                              |
|--------------------------|----------------------------------------------------------------------|
| Đinh Trọng Mạnh Quân     | Thiết kế mô hình, lập trình UNO 1 & UNO 2, tích hợp hệ thống.        |
| Trần Mạnh Hùng           | Lập trình UNO 3, truyền thông UART liên bo mạch, LCD & còi báo.      |
| Trương Văn Phúc          | Kiểm thử thực nghiệm, thu thập dữ liệu & hoàn thiện tài liệu.       |

---

*Dự án được thực hiện trong khuôn khổ bài tập lớn / đánh giá học phần Hệ thống nhúng.*
