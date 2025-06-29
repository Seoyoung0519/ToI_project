#ifndef DUMMY_MOTION_DATA_H
#define DUMMY_MOTION_DATA_H

// 더미 메시지들 (BLE 메시지 형식과 동일)
static const char* dummy_data[] = {
    "Activity:high Score:610.5",
    "Activity:medium Score:512.7",
    "Activity:high Score:627.4",
    "Activity:low Score:252.1",
    "Activity:high Score:730.2", 
    "Activity:high Score:640.3"
};

#define DUMMY_DATA_COUNT (sizeof(dummy_data) / sizeof(dummy_data[0]))

#endif // DUMMY_MOTION_DATA_H