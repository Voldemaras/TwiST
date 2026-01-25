/* ============================================================================
 * TwiST Framework | Core Framework
 * ============================================================================
 * @file      EventBus.h
 * @brief     Publish/subscribe event bus for loosely-coupled component communication
 *
 * ARCHITECTURE ROLE:
 * - Layer:        Core
 * - Type:         Core Service
 * - Hardware:     None
 * - Implements:   None (core infrastructure)
 *
 * PRINCIPLES:
 * - Enables loose coupling through event-driven architecture
 * - Synchronous and asynchronous event publishing
 * - Priority-based listener execution
 * - No dependencies on other framework components
 * - Zero heap allocation for event queue
 *
 * CAPABILITIES:
 * - Subscribe/unsubscribe to named events
 * - Publish events immediately (synchronous)
 * - Queue events for later processing (asynchronous)
 * - Priority-based event routing
 * - Event statistics and monitoring
 *
 * AUTHOR:    Voldemaras Birskys
 * EMAIL:     voldemaras@gmail.com
 * PROJECT:   TwiST Framework
 * VERSION:   1.0
 * ============================================================================
 */

#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Maximum number of event listeners
#ifndef MAX_EVENT_LISTENERS
#define MAX_EVENT_LISTENERS 32
#endif

// Maximum number of queued events
#ifndef MAX_EVENT_QUEUE
#define MAX_EVENT_QUEUE 16
#endif

// Event priority
enum EventPriority {
    PRIORITY_LOW = 0,
    PRIORITY_NORMAL = 10,
    PRIORITY_HIGH = 20,
    PRIORITY_CRITICAL = 30
};

// Event structure
struct Event {
    const char* name;           // Event name (e.g., "device.state.changed")
    uint16_t sourceDeviceId;    // Source device ID (0 = system)
    JsonDocument* data;         // Event payload (can be NULL)
    EventPriority priority;
    unsigned long timestamp;
};

// Event listener callback
typedef void (*EventListener)(const Event& event);

// Internal listener registration
struct EventSubscription {
    uint16_t id;
    const char* eventName;
    EventListener callback;
    EventPriority priority;
    bool active;
};

/**
 * @brief Event bus for publish/subscribe messaging
 *
 * The EventBus enables loose coupling between components through events.
 * Components can publish events and subscribe to events without knowing
 * about each other.
 *
 * Example events:
 * - "device.state.changed" - Device state changed
 * - "servo.move.complete" - Servo finished moving
 * - "joystick.calibrated" - Joystick calibration completed
 * - "bridge.mapping.added" - New bridge mapping created
 *
 * Example usage:
 * ```cpp
 * EventBus eventBus;
 *
 * // Subscribe to events
 * eventBus.subscribe("servo.move.complete", [](const Event& evt) {
 *   Serial.print("Servo ");
 *   Serial.print(evt.sourceDeviceId);
 *   Serial.println(" finished moving");
 * });
 *
 * // Publish event
 * Event evt = {
 *   .name = "servo.move.complete",
 *   .sourceDeviceId = 100,
 *   .data = NULL,
 *   .priority = PRIORITY_NORMAL
 * };
 * eventBus.publish(evt);
 * ```
 */
class EventBus {
public:
    EventBus();
    ~EventBus();

    // ===== Subscription =====

    /**
     * @brief Subscribe to an event
     * @param eventName Event name to subscribe to (e.g., "device.state.changed")
     * @param listener Callback function to call when event is published
     * @param priority Priority level (higher priority listeners called first)
     * @return Listener ID (for unsubscribing) or 0 if failed
     */
    uint16_t subscribe(const char* eventName, EventListener listener, EventPriority priority = PRIORITY_NORMAL);

    /**
     * @brief Unsubscribe a specific listener
     * @param listenerId Listener ID returned by subscribe()
     */
    void unsubscribe(uint16_t listenerId);

    /**
     * @brief Unsubscribe all listeners for an event
     * @param eventName Event name
     */
    void unsubscribeAll(const char* eventName);

    // ===== Publishing =====

    /**
     * @brief Publish an event immediately (synchronous)
     * @param event Event structure
     *
     * Calls all matching listeners immediately in order of priority.
     */
    void publish(const Event& event);

    /**
     * @brief Publish an event asynchronously (queued)
     * @param event Event structure
     *
     * Event is added to queue and will be processed during processEvents().
     */
    void publishAsync(const Event& event);

    // ===== Processing =====

    /**
     * @brief Process queued asynchronous events
     *
     * Call this in the main loop to process events published via publishAsync().
     */
    void processEvents();

    /**
     * @brief Get number of events pending in queue
     * @return Pending event count
     */
    uint16_t getPendingEventCount() const;

    // ===== Statistics =====

    /**
     * @brief Get total number of events published (lifetime)
     * @return Total event count
     */
    unsigned long getEventCount() const;

    /**
     * @brief Get number of active listeners
     * @return Listener count
     */
    unsigned long getListenerCount() const;

private:
    EventSubscription _listeners[MAX_EVENT_LISTENERS];
    uint8_t _listenerCount;
    uint16_t _nextListenerId;

    Event _eventQueue[MAX_EVENT_QUEUE];
    uint8_t _queueHead;
    uint8_t _queueTail;
    uint8_t _queueSize;

    unsigned long _totalEventCount;

    // Helper to check if event name matches subscription
    bool eventMatches(const char* eventName, const char* pattern);

    // Helper to trigger listeners for an event
    void triggerListeners(const Event& event);
};

#endif // EVENT_BUS_H