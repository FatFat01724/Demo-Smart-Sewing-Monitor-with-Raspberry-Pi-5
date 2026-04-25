from django.urls import path
from . import views

urlpatterns = [
    path('api/data/', views.receive_data), # Link để Pi gửi data: http://127.0.0.1:8000/api/data/
    path('dashboard/', views.dashboard),  # Link xem web: http://127.0.0.1:8000/dashboard/
    path('api/chatbot/', views.chatbot),  # API cho chatbot
    path('dashboard-data/', views.dashboard_data),  # API cho auto-update dashboard
    path('api/machine-history/<str:machine_name>/', views.machine_history, name='machine_history'),
]