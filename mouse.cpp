#include "mouse.h"
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

Mouse::Mouse(int _gadget_fd, int _input_fd) {
    gadget_fd = _gadget_fd;
    input_fd = _input_fd;
}

void Mouse::move(MouseData _data) {
    move(_data.tip, _data.x, _data.y, _data.wheel);
}

void Mouse::move(uint8_t tip, int x, int y, int wheel) {
    struct MouseReport report;
    report.tip_switch = tip;
    report.x_lsb = (x & 0xff);
    report.x_msb = (x >> 8);
    report.y_lsb = (y & 0xff);
    report.y_msb = (y >> 8);
    report.wheel_lsb = (wheel & 0xff);
    report.wheel_msb = (wheel >> 8);

    ssize_t bytesWritten = write(gadget_fd, &report, sizeof(struct MouseReport));
    if (bytesWritten == -1) {
        std::cerr << "Mouse: Error writing" << std::endl;
        close(gadget_fd);
    }
}

bool Mouse::update() {
    struct input_event ev;

    ssize_t bytesRead = read(input_fd, &ev, sizeof(struct input_event));
    if (bytesRead == -1) {
        std::cerr << "Error reading" << std::endl;
    }

    if (bytesRead == sizeof(struct input_event)) {
        if (ev.type == EV_KEY) {

            std::cout << "KEY: " << std::hex << ev.code << std::endl;

            if (ev.code == BTN_LEFT) {
                data.tip = ev.value == 1 ? data.tip | (uint8_t)ClickType::LEFT : data.tip ^ (uint8_t)ClickType::LEFT;
            } else if (ev.code == BTN_RIGHT)
                data.tip = ev.value == 1 ? data.tip | (uint8_t)ClickType::RIGHT : data.tip ^ (uint8_t)ClickType::RIGHT;
            
            else if (ev.code == BTN_MIDDLE)
                data.tip = ev.value == 1 ? data.tip | (uint8_t)ClickType::MIDDLE : data.tip ^ (uint8_t)ClickType::MIDDLE;
            
            else if (ev.code == BTN_FORWARD || ev.code == BTN_EXTRA)
                data.tip = ev.value == 1 ? data.tip | (uint8_t)ClickType::FORWARD : data.tip ^ (uint8_t)ClickType::FORWARD;
            else if (ev.code == BTN_BACK || ev.code == BTN_SIDE)
                data.tip = ev.value == 1 ? data.tip | (uint8_t)ClickType::BACK : data.tip ^ (uint8_t)ClickType::BACK;
            else 
                std::cout << "KEY: " << std::hex << ev.code << std::endl;

        } else if (ev.type == EV_REL) {

            if (ev.code == REL_X)
                currentdata.x = ev.value;
            
            else if (ev.code == REL_Y)
                currentdata.y = ev.value;
            
            else if (ev.code == REL_WHEEL)
                currentdata.wheel = ev.value;

        } else if (ev.type == EV_SYN && ev.code == SYN_REPORT) {
            data.x = currentdata.x;
            data.y = currentdata.y;
            data.wheel = currentdata.wheel;
            currentdata = {};
            return true;
        }
    }

    return false;
}

MouseData Mouse::get_data() {
    return data;
}