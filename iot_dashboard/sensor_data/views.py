from django.shortcuts import render

# Create your views here.
from rest_framework.decorators import api_view
from rest_framework.response import Response
from .models import SensorReading
from django.conf import settings
from groq import Groq

# Khởi tạo Groq client
client = Groq(api_key=settings.GROQ_API_KEY)

# 1. API để Raspberry Pi gửi dữ liệu lên (POST)
@api_view(['POST'])
def receive_data(request):
    data = request.data
    SensorReading.objects.create(
        sensor_name=data.get('sensor_name'),
        stitch_count=data.get('stitch_count', 0),
        stage=data.get('stage', 'Idle'),
        pattern_name=data.get('pattern_name', 'Unknown'),
        spm=data.get('spm', 0),
        run_ratio=data.get('run_ratio', 0.0),
        evib_var=data.get('evib_var', 0.0),
        ecur_var=data.get('ecur_var', 0.0),
    )
    return Response({"status": "Data saved!"}, status=201)

# 2. Trang Dashboard để xem dữ liệu (GET)
def dashboard(request):
    from django.utils import timezone
    from datetime import timedelta
    
    # Lấy dữ liệu mới nhất cho mỗi máy may (1-10)
    readings = []
    for i in range(1, 9):  # Sewing Machine 1 đến 10
        machine_name = f"Sewing Machine {i}"
        latest_reading = SensorReading.objects.filter(
            sensor_name=machine_name
        ).order_by('-timestamp').first()
        readings.append(latest_reading)

    # Tính tổng số mũi may từ 10 máy (chỉ lấy reading mới nhất của mỗi máy)
    total_stitches = sum([r.stitch_count for r in readings if r is not None])
    
    # Đếm máy online (có dữ liệu)
    active_machines = sum([1 for r in readings if r is not None])
    
    # Tính trung bình OEE
    avg_oee = 0
    # if active_machines > 0:
    #     total_oee = sum([r.get_oee() for r in readings if r is not None])
    #     avg_oee = round(total_oee / active_machines, 1)
    
    # Tính trung bình SPM
    avg_spm = 0
    if active_machines > 0:
        total_spm = sum([r.spm for r in readings if r is not None])
        avg_spm = round(total_spm / active_machines, 0)
    
    # Phát hiện warning (máy có vấn đề)
    warnings = []
    for r in readings:
        if r is not None:
            if r.evib_var > 0.7 or r.ecur_var > 0.7:
                warnings.append({
                    'machine': r.sensor_name,
                    'issue': f"Vibration: {r.evib_var:.2f}, Current: {r.ecur_var:.2f}",
                    'severity': 'high' if (r.evib_var > 0.85 or r.ecur_var > 0.85) else 'medium'
                })

    context = {
        'readings': readings,
        'total_stitches': total_stitches,
        'active_machines': active_machines,
        'total_machines': 8,
        'avg_oee': avg_oee,
        'avg_spm': int(avg_spm),
        'warnings': warnings,
    }
    return render(request, 'sensor_data/dashboard.html', context)

# 3. API cho AI Chatbot sử dụng Groq
@api_view(['POST'])
def chatbot(request):
    user_message = request.data.get('message', '')
    
    # Lấy dữ liệu từ database để tạo context
    recent_readings = SensorReading.objects.order_by('-timestamp')[:16]  # Lấy 16 bản ghi gần nhất
    
    # Tạo context từ dữ liệu máy may
    context = "Dữ liệu máy may gần nhất:\n"
    for r in recent_readings:
        context += f"- Máy {r.sensor_name}: Công đoạn '{r.pattern_name}', SPM: {r.spm}, Run ratio: {r.run_ratio/100:.1%}, Trạng thái: {r.get_status()}, Vibration: {r.evib_var:.2f}, Current: {r.ecur_var:.2f}, Thời gian: {r.timestamp.strftime('%H:%M %d/%m')}\n"
    
    # Tạo prompt cho Groq
    prompt = f"""
Bạn là một Robot xử lý dữ liệu IoT có tư duy toán học cực kỳ chính xác. 
Nhiệm vụ: So sánh [Current] và [Vibration] với ngưỡng CỐ ĐỊNH là 0.80.

DỮ LIỆU ĐẦU VÀO:
{context}

TRUY VẤN: 
"{user_message}"

---
QUY TRÌNH XỬ LÝ DỮ LIỆU (BẮT BUỘC):
1. Đọc giá trị Current và Vibration của máy được hỏi.
2. Kiểm tra Logic toán học:
   - Nếu (Current >= 0.8) HOẶC (Vibration >= 0.8) -> Gán nhãn: [TRẠNG THÁI NGUY HIỂM].
   - Nếu (Current < 0.8) VÀ (Vibration < 0.8) -> Gán nhãn: [TRẠNG THÁI AN TOÀN].

QUY TẮC HIỂN THỊ DÀNH CHO [TRẠNG THÁI AN TOÀN]:
- Icon: ✅
- TUYỆT ĐỐI KHÔNG xuất hiện tiêu đề "🚨 CẢNH BÁO KHẨN CẤP".
- Mục Checklist: Chỉ ghi duy nhất "- [x] Thông số an toàn".

QUY TẮC HIỂN THỊ DÀNH CHO [TRẠNG THÁI NGUY HIỂM]:
- Icon: ⚠️
- BẮT BUỘC xuất hiện tiêu đề "🚨 CẢNH BÁO KHẨN CẤP".
- Chỉ liệt kê thông số nào CÓ GIÁ TRỊ >= 0.8. Thông số < 0.8 không được liệt kê vào mục cảnh báo.
- Mục Checklist: 
    + Nếu Current >= 0.8: "- Kiểm tra vải\n- Motor\n- Nguồn điện"
    + Nếu Vibration >= 0.8: "- Kiểm tra cơ khí\n- Độ rung sàn\n- Bôi trơn"

---
ĐỊNH DẠNG TRẢ VỀ (MARKDOWN):

### 📊 PHÂN TÍCH NHANH
- **[Tên máy]**: [Icon]
- SPM: **[Giá trị]**
- Run Ratio: **[Giá trị]%**
- Current: **[Giá trị]**
- Vibration: **[Giá trị]**

[KHỐI NÀY CHỈ XUẤT HIỆN NẾU [TRẠNG THÁI NGUY HIỂM]]
> ### 🚨 CẢNH BÁO KHẨN CẤP
> Máy [Tên máy] vận hành vượt ngưỡng an toàn tại:
> - **[Tên thông số >= 0.8]**: [Giá trị]

### 🛠️ CHECKLIST HÀNH ĐỘNG
[Nội dung tương ứng với nhãn trạng thái]

---
LƯU Ý CỰC TRỌNG: 
- 0.80 là ngưỡng bắt đầu báo động. 0.79 là tuyệt đối an toàn.
- Không chào hỏi, không giải thích logic. Trả về kết quả trực tiếp.
"""
    
    try:
        completion = client.chat.completions.create(
            model="openai/gpt-oss-120b",
            messages=[
                {
                    "role": "system",
                    "content": "Bạn là trợ lý thông minh chuyên phân tích dữ liệu vận hành máy may và đưa ra gợi ý tối ưu sản xuất.",
                },
                {
                    "role": "user",
                    "content": prompt,
                },
            ],
            temperature=0.0,
        )
        ai_response = completion.choices[0].message.content.strip()
    except Exception as e:
        ai_response = f"Lỗi kết nối với Groq: {str(e)}. Vui lòng kiểm tra API key."
    
    return Response({"response": ai_response})

# 4. API để lấy dữ liệu dashboard cho auto-update
@api_view(['GET'])
def dashboard_data(request):
    from django.utils import timezone
    from datetime import timedelta
    
    # Lấy dữ liệu mới nhất cho mỗi máy may
    readings = []
    for i in range(1, 9):  # Sewing Machine 1 đến 8
        machine_name = f"Sewing Machine {i}"
        latest_reading = SensorReading.objects.filter(
            sensor_name=machine_name
        ).order_by('-timestamp').first()
        if latest_reading:
            readings.append({
                'sensor_name': latest_reading.sensor_name,
                'stitch_count': latest_reading.stitch_count,
                'stage': latest_reading.stage,
                'pattern_name': latest_reading.pattern_name,
                'spm': latest_reading.spm,
                'run_ratio': latest_reading.run_ratio,
                'evib_var': latest_reading.evib_var,
                'ecur_var': latest_reading.ecur_var,
                'timestamp': latest_reading.timestamp,
                'status': latest_reading.get_status(),
                'oee': latest_reading.get_oee()
            })

    # Tính tổng số mũi may
    total_stitches = sum([r['stitch_count'] for r in readings])
    
    # Đếm máy online
    active_machines = len(readings)
    
    # Tính trung bình SPM
    avg_spm = 0
    if active_machines > 0:
        total_spm = sum([r['spm'] for r in readings])
        avg_spm = round(total_spm / active_machines, 0)
    
    # Phát hiện warning
    warnings = []
    for r in readings:
        if r['evib_var'] > 0.7 or r['ecur_var'] > 0.7:
            warnings.append({
                'machine': r['sensor_name'],
                'issue': f"Vibration: {r['evib_var']:.2f}, Current: {r['ecur_var']:.2f}",
                'severity': 'high' if (r['evib_var'] > 0.85 or r['ecur_var'] > 0.85) else 'medium'
            })

    data = {
        'readings': readings,
        'total_stitches': total_stitches,
        'active_machines': active_machines,
        'total_machines': 8,
        'avg_spm': int(avg_spm),
        'warnings': warnings,
    }
    return Response(data)

@api_view(['GET'])
def machine_history(request, machine_name):
    history_queryset = SensorReading.objects.filter(
        sensor_name=machine_name
    ).order_by('-timestamp')[:20]
    
    history = list(reversed(history_queryset)) 
    
    data = {
        'ecur_values': [float(r.ecur_var) for r in history],
        'evib_values': [float(r.evib_var) for r in history],
    }
    return Response(data)