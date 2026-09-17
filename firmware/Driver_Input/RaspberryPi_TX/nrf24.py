"""nRF24L01 레지스터 레벨 드라이버 (bring-up 전용).

STM32 Driver_Input 쪽 코드(firmware/Driver_Input/Core/Src/main.c)와 동일한
커맨드/레지스터 상수를 쓴다. 라이브러리(RF24 등)를 쓰지 않고 spidev로 직접
R_REGISTER/W_REGISTER를 보내는 이유는 송수신 양쪽에서 같은 레벨의 원시 통신을
확인하기 위함이다 - 나중에 패킷 레이어를 얹을 때도 이 위에 그대로 쌓는다.

핀 배정은 아직 TBD (배선 전). CE는 GPIO로 별도 제어하고, CSN은 spidev의
하드웨어 CS0(GPIO8)를 그대로 쓴다.
"""

import time

import spidev
from gpiozero import DigitalOutputDevice

# --- nRF24L01 커맨드 (STM32 쪽과 동일) ---
CMD_R_REGISTER = 0x00
CMD_W_REGISTER = 0x20
CMD_NOP = 0xFF

# --- 자주 쓰는 레지스터 주소 ---
REG_CONFIG = 0x00
REG_STATUS = 0x07

# CE 핀: TBD, 배선 확정 전까지 임시값. BCM 번호 기준.
CE_GPIO_BCM = 22


class NRF24:
    def __init__(self, bus: int = 0, device: int = 0, ce_pin: int = CE_GPIO_BCM,
                 max_speed_hz: int = 1_000_000):
        self._spi = spidev.SpiDev()
        self._spi.open(bus, device)
        self._spi.max_speed_hz = max_speed_hz
        self._spi.mode = 0b00  # nRF24L01: CPOL=0, CPHA=0 (STM32 SPI1과 동일)

        self._ce = DigitalOutputDevice(ce_pin, initial_value=False)

    def close(self) -> None:
        self._ce.off()
        self._spi.close()

    def read_register(self, reg: int) -> int:
        """R_REGISTER: 두 번째 응답 바이트가 레지스터 값, 첫 번째는 STATUS."""
        tx = [CMD_R_REGISTER | (reg & 0x1F), CMD_NOP]
        rx = self._spi.xfer2(tx)
        return rx[1]

    def write_register(self, reg: int, value: int) -> int:
        """W_REGISTER. 반환값은 트랜잭션 첫 바이트로 온 STATUS."""
        tx = [CMD_W_REGISTER | (reg & 0x1F), value & 0xFF]
        rx = self._spi.xfer2(tx)
        return rx[0]

    def read_status(self) -> int:
        """NOP 커맨드로 STATUS만 읽는다 (레지스터 접근 없이)."""
        rx = self._spi.xfer2([CMD_NOP])
        return rx[0]


def bringup_check() -> None:
    """STM32 쪽 T-RF-000 이후 bench 시험과 동일한 SPI read/write 확인.

    모듈 미연결 상태에서는 STM32 쪽과 마찬가지로 0xFF만 나오는 게 정상이다
    (MISO floating). 실제 모듈을 연결한 뒤 STATUS 기본값(약 0x0E)과
    CONFIG 기본값(약 0x08)이 보이는지, write 후 값이 실제로 바뀌는지 확인한다.
    """
    nrf = NRF24()
    try:
        nrf._ce.off()
        time.sleep(0.1)  # nRF24L01 전원 안정화 대기 (STM32 쪽과 동일)

        status_before = nrf.read_register(REG_STATUS)
        config_before = nrf.read_register(REG_CONFIG)

        nrf.write_register(REG_CONFIG, 0x0A)
        config_after = nrf.read_register(REG_CONFIG)

        print(
            f"NRF24 STATUS=0x{status_before:02X} "
            f"CONFIG(before)=0x{config_before:02X} "
            f"CONFIG(after write 0x0A)=0x{config_after:02X}"
        )
    finally:
        nrf.close()


if __name__ == "__main__":
    bringup_check()
