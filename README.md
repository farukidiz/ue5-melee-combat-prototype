\# UE5 Melee Combat Prototype



A third-person melee combat prototype developed in Unreal Engine 5, focused on combat system design, target selection, combo flow, and gameplay responsiveness.



This project extends a course-based foundation with custom gameplay systems and architecture improvements.



\---



\## 🎥 Showcase



Video: https://youtu.be/ugWEUXH9Re8



\---



\## 🔥 Core Features



\- Dynamic target selection system with multi-factor scoring

\- Context-aware attack selection system

\- Responsive combo attack flow

\- Motion warping with target alignment

\- Enemy AI with patrol, chase, attack, and hit states

\- Breakable actors with weighted loot spawning

\- Pickup system (gold, souls, potions)



\---



\## 🧠 Technical Highlights



\### Target Selection System

The targeting system evaluates potential targets using multiple criteria instead of relying solely on distance.



Factors include:

\- distance

\- facing direction

\- camera alignment

\- screen-center bias



This results in more intuitive and player-focused targeting behavior.



\---



\### Combat Attack System

Attacks are selected dynamically based on combat context:



\- close-range combo continuation

\- distance-based attack filtering

\- fallback free attacks when no valid target is available



The system is managed through a modular combat attack component and data-driven attack definitions.



\---



\### Motion Warping Integration

Motion warping is used to improve attack alignment and ensure better visual coherence between character and target.



\---



\### Enemy AI System

Enemy behavior is controlled via a custom AI controller with state-driven logic:



\- patrolling

\- chasing

\- attacking

\- hit reaction

\- death



State transitions are evaluated dynamically based on distance and target validity.



\---



\### Breakable and Loot System

Breakable actors spawn loot based on weighted data table configuration, allowing flexible and data-driven drop behavior.



\---



\## 🧩 Project Structure



Key systems are organized as follows:



\- `CombatTargetingComponent`  

&#x20; Handles candidate scanning, target scoring, and active target selection



\- `CombatAttackComponent`  

&#x20; Manages attack selection, combo flow, and attack state



\- `SlashCharacter`  

&#x20; Main player logic and combat input handling



\- `EnemyAIController`  

&#x20; Controls enemy state transitions and behavior



\- `BreakableActor`  

&#x20; Handles destruction and loot spawning



\---



\## ⚠️ Notes



This repository focuses on gameplay systems and code architecture.



Due to project size and asset constraints, content assets are not included.  

Please refer to the showcase video for full gameplay demonstration.



\---



\## 🛠️ How to Use



1\. Clone the repository

2\. Open the `.uproject` file

3\. Generate project files if needed

4\. Build using Visual Studio 2022



## Personal Note

This project was built to explore combat system design and improve my understanding of gameplay architecture in Unreal Engine. I focused especially on targeting logic, combo flow, and responsiveness.
