void Controller_Tracking_Calculate() {
    // 1. 패킷 데이터 가져오기 (비트필드 구조체 매핑)
    SpiPacket_t *packet = (SpiPacket_t *)spiRxBuffer;

    // 2. 상태 변화 감지 (Edge Detection)
    // [감지 비트 변화 감지]
    if (trackingData.isDetected == 0 && packet->isDetected == 1) {
        // 이전에 없었는데 지금 생겼다면? -> TARGET_ON!
        osMessagePut(trackingEventMsgBox, EVENT_TARGET_ON, 0);
    } 
    else if (trackingData.isDetected == 1 && packet->isDetected == 0) {
        // 있었는데 없어졌다면? -> TARGET_LOST!
        osMessagePut(trackingEventMsgBox, EVENT_TARGET_LOST, 0);
    }

    // [조준 비트 변화 감지]
    if (trackingData.isAimed == 0 && packet->isAimed == 1) {
        // 조준이 완료된 시점에 딱 한 번! -> TARGET_AIMED!
        osMessagePut(trackingEventMsgBox, EVENT_TARGET_AIMED, 0);
    }

    // 3. 현재 정보를 이전 정보로 업데이트 (다음 루프 비교용)
    trackingData.isDetected = packet->isDetected;
    trackingData.isAimed = packet->isAimed;
    trackingData.x_pos = packet->offsetX;
    trackingData.y_pos = packet->offsetY;

    // 4. 각도 계산 및 전송 (Follow 상태일 때만 의미가 있겠죠?)
    Controller_Tracking_ComputeServoAngle();
}