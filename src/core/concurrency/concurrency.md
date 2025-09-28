# Concurrency in Engine

A Job System will maintain a job queue with a pool of threads, scheduling jobs to available threads from the job queue.

An example of this approach is like:

CPU0 | CPU1 |  
-----| ---- |
HID  |     | 
Update GameObjects |  |
Spawn Animation jobs | Pose blending |
Post animation gameobject update |
Spawn dynamics jobs | Physics sim |
Kick rendering | Visibility |

In a game loop, it would look like:

```cpp

while (game.should_close()) {
    poll_HID();

    update_entities(&game_objects);

    JobPool::kickJob(Animation::blend_pose, Priority::HIGH);

    
}

```