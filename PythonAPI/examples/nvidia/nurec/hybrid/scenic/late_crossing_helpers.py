"""Event records shared by the late-crossing Scenic behaviors and validator."""
EVENTS = []


def reset_events():
    EVENTS.clear()


def mark_event(name, time, **details):
    if not any(e['name'] == name for e in EVENTS):
        EVENTS.append(dict(name=name, time_s=float(time), **details))


def event_time(name):
    return next((e['time_s'] for e in EVENTS if e['name'] == name), None)
