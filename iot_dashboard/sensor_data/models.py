from django.db import models

# Create your models here.
class SensorReading(models.Model):  
    sensor_name = models.CharField(max_length=100)
    stitch_count = models.IntegerField()  # Số mũi may
    stage = models.CharField(max_length=100)  # Công đoạn đang may
    
    # Thông số AI/IoT từ model
    pattern_name = models.CharField(max_length=100, default="Unknown")  # Tên công đoạn chi tiết
    spm = models.IntegerField(default=0)  # Stitches Per Minute
    run_ratio = models.FloatField(default=0.0)  # Tỷ lệ hoạt động (0-1)
    evib_var = models.FloatField(default=0.0)  # Biến thiên rung động
    ecur_var = models.FloatField(default=0.0)  # Biến thiên dòng điện
    
    timestamp = models.DateTimeField(auto_now_add=True) # Tự động lấy giờ hiện tại

    def __str__(self):
        return f"{self.sensor_name} - {self.timestamp}"
    
    def get_status(self):
        """Xác định trạng thái máy dựa trên thông số"""
        if self.run_ratio < 2:
            return 'idle'  # Máy không hoạt động
        elif self.evib_var > 0.7 or self.ecur_var > 0.7:
            return 'warning'  # Phát hiện bất thường
        elif self.run_ratio > 70:
            return 'running'  # Máy chạy bình thường
        else:
            return 'slow'  # Máy chạy chậm/bị gián đoạn
    
    def get_oee(self):
        """Tính OEE (Overall Equipment Effectiveness) = run_ratio * 100"""
        return round(self.run_ratio * 100, 1)