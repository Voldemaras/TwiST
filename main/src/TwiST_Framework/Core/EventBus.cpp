#include "EventBus.h"
#include <string.h>

EventBus::EventBus()
    : _listenerCount(0),
      _nextListenerId(1),
      _queueHead(0),
      _queueTail(0),
      _queueSize(0),
      _totalEventCount(0) {

    // Initialize listeners
    for (uint8_t i = 0; i < MAX_EVENT_LISTENERS; i++) {
        _listeners[i].id = 0;
        _listeners[i].eventName = NULL;
        _listeners[i].callback = NULL;
        _listeners[i].priority = PRIORITY_NORMAL;
        _listeners[i].active = false;
    }

    // Initialize event queue
    for (uint8_t i = 0; i < MAX_EVENT_QUEUE; i++) {
        _eventQueue[i].name = NULL;
        _eventQueue[i].data = NULL;
    }

    Serial.println("[EventBus] Initialized");
}

EventBus::~EventBus() {
    Serial.println("[EventBus] Shutting down");
}

// ===== Subscription =====

uint16_t EventBus::subscribe(const char* eventName, EventListener listener, EventPriority priority) {
    if (eventName == NULL || listener == NULL) {
        Serial.println("[EventBus ERROR] Cannot subscribe with NULL eventName or listener");
        return 0;
    }

    if (_listenerCount >= MAX_EVENT_LISTENERS) {
        Serial.println("[EventBus ERROR] Listener limit reached");
        return 0;
    }

    // Find empty slot
    for (uint8_t i = 0; i < MAX_EVENT_LISTENERS; i++) {
        if (!_listeners[i].active) {
            _listeners[i].id = _nextListenerId++;
            _listeners[i].eventName = eventName;
            _listeners[i].callback = listener;
            _listeners[i].priority = priority;
            _listeners[i].active = true;
            _listenerCount++;

            Serial.print("[EventBus] Subscribed to '");
            Serial.print(eventName);
            Serial.print("' (ID: ");
            Serial.print(_listeners[i].id);
            Serial.println(")");

            return _listeners[i].id;
        }
    }

    return 0;
}

void EventBus::unsubscribe(uint16_t listenerId) {
    for (uint8_t i = 0; i < MAX_EVENT_LISTENERS; i++) {
        if (_listeners[i].active && _listeners[i].id == listenerId) {
            Serial.print("[EventBus] Unsubscribed listener ID: ");
            Serial.println(listenerId);

            _listeners[i].active = false;
            _listeners[i].id = 0;
            _listeners[i].eventName = NULL;
            _listeners[i].callback = NULL;
            _listenerCount--;
            return;
        }
    }
}

void EventBus::unsubscribeAll(const char* eventName) {
    if (eventName == NULL) {
        return;
    }

    Serial.print("[EventBus] Unsubscribing all from '");
    Serial.print(eventName);
    Serial.println("'");

    for (uint8_t i = 0; i < MAX_EVENT_LISTENERS; i++) {
        if (_listeners[i].active && strcmp(_listeners[i].eventName, eventName) == 0) {
            _listeners[i].active = false;
            _listeners[i].id = 0;
            _listeners[i].eventName = NULL;
            _listeners[i].callback = NULL;
            _listenerCount--;
        }
    }
}

// ===== Publishing =====

void EventBus::publish(const Event& event) {
    if (event.name == NULL) {
        return;
    }

    _totalEventCount++;
    triggerListeners(event);
}

void EventBus::publishAsync(const Event& event) {
    if (event.name == NULL) {
        return;
    }

    if (_queueSize >= MAX_EVENT_QUEUE) {
        Serial.println("[EventBus WARNING] Event queue full, dropping event");
        return;
    }

    // Add event to queue
    _eventQueue[_queueTail] = event;
    _eventQueue[_queueTail].timestamp = millis();

    _queueTail = (_queueTail + 1) % MAX_EVENT_QUEUE;
    _queueSize++;
}

// ===== Processing =====

void EventBus::processEvents() {
    while (_queueSize > 0) {
        // Get event from queue
        Event event = _eventQueue[_queueHead];
        _queueHead = (_queueHead + 1) % MAX_EVENT_QUEUE;
        _queueSize--;

        // Process event
        _totalEventCount++;
        triggerListeners(event);
    }
}

uint16_t EventBus::getPendingEventCount() const {
    return _queueSize;
}

// ===== Statistics =====

unsigned long EventBus::getEventCount() const {
    return _totalEventCount;
}

unsigned long EventBus::getListenerCount() const {
    return _listenerCount;
}

// ===== Private Helpers =====

bool EventBus::eventMatches(const char* eventName, const char* pattern) {
    if (eventName == NULL || pattern == NULL) {
        return false;
    }

    // For now, exact match only
    // TODO: Support wildcards like "device.*" or "servo.+.complete"
    return strcmp(eventName, pattern) == 0;
}

void EventBus::triggerListeners(const Event& event) {
    // Find matching listeners, prioritize by priority level
    // Note: In a full implementation, we'd sort listeners by priority
    // For simplicity, we'll iterate in priority order

    for (EventPriority p = PRIORITY_CRITICAL; p >= PRIORITY_LOW; p = (EventPriority)(p - 10)) {
        for (uint8_t i = 0; i < MAX_EVENT_LISTENERS; i++) {
            if (_listeners[i].active &&
                _listeners[i].priority == p &&
                eventMatches(event.name, _listeners[i].eventName)) {

                // Call listener callback
                if (_listeners[i].callback) {
                    _listeners[i].callback(event);
                }
            }
        }
    }
}