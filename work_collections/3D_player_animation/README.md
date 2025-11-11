# About
this is demonstration on how to chage animation according to input control with frame interpolation and state management
- press e to equip bow
- press i to walk forward
- press wasd to move camera
- left click to draw the bow
# state machine
```mermaid
stateDiagram-v2
    [*] --> IDLE

    IDLE --> IDLE_WALK : Press FORWARD_KEY
    IDLE --> IDLE_EQUIP : Press 'E'
    IDLE --> IDLE_SHOOT : Press Left Mouse

    IDLE_WALK --> WALK : blend > 0.9
    WALK --> WALK_IDLE : Release FORWARD_KEY
    WALK --> WALK_RUN : Press Left Control
    WALK_IDLE --> IDLE : blend > 0.9

    WALK_RUN --> RUN : blend > 0.9
    RUN --> RUN_WALK : Release FORWARD_KEY
    RUN_WALK --> WALK_IDLE : blend > 0.9

    IDLE_EQUIP --> EQUIP_IDLE : blend > 0.9
    EQUIP_IDLE --> IDLE : blend > 0.9

    IDLE_SHOOT --> SHOOT_IDLE : blend > 0.95
    SHOOT_IDLE --> IDLE : blend > 0.9
```
# Demo
https://github.com/user-attachments/assets/30a60bc2-8804-4052-b27a-609ccc6557fc
# Credit
all this asset are from https://www.mixamo.com/ with 
- Model: Erika Archer
- Animation: Lite Long Bow Pack
