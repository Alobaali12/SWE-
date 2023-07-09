import sys
from PyQt5.QtWidgets import QApplication, QWidget
from PyQt5.QtGui import QPainter, QColor, QPolygonF, QTransform
from PyQt5.QtCore import Qt, QTimer, QRectF
import serial
from pyquaternion import Quaternion

class IMUGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setGeometry(100, 100, 400, 400)
        self.setWindowTitle('IMU GUI')

        self.quaternion = Quaternion()

        self.serial_port = serial.Serial('COM3', 115200)  # Update with the correct port

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_rotation)
        self.timer.start(10)  # Adjust interval based on your requirements

    def update_rotation(self):
        if self.serial_port.is_open and self.serial_port.in_waiting > 0:
            data = self.serial_port.readline().decode().strip()
            if data.startswith('Time'):
                values = data.split(',')
                if len(values) >= 5:
                    q_w = float(values[1])
                    q_x = float(values[2])
                    q_y = float(values[3])
                    q_z = float(values[4])

                    # Create a quaternion from the received values
                    self.quaternion = Quaternion(q_w, q_x, q_y, q_z)

        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.translate(self.width() // 2, self.height() // 2)

        # Draw a simple representation of the IMU orientation
        side = min(self.width(), self.height()) * 0.4
        rect = QRectF(-side / 2, -side / 2, side, side)
        painter.setPen(Qt.NoPen)
        painter.setBrush(QColor(200, 200, 200))
        painter.drawEllipse(rect)

        painter.setPen(QColor(0, 0, 0))
        painter.drawLine(rect.topLeft(), rect.bottomRight())
        painter.drawLine(rect.topRight(), rect.bottomLeft())

        # Convert the quaternion orientation to Euler angles
        euler_angles = self.quaternion.euler_angles()

        # Rotate the painter based on the Euler angles
        rotation_x = euler_angles[0] * 180 / 3.14159265359
        rotation_y = euler_angles[1] * 180 / 3.14159265359
        rotation_z = euler_angles[2] * 180 / 3.14159265359
        painter.rotate(rotation_x, rotation_y, rotation_z)

        # Draw the rotated line representing the IMU orientation
        painter.setPen(QColor(255, 0, 0))
        painter.drawLine(-side / 2, 0, side / 2, 0)





if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = IMUGUI()
    window.show()
    sys.exit(app.exec_())
