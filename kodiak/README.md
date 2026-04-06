# External AimAssist

This repository is the source code for an external aim assist for **Minecraft Bedrock Edition** version **1.21.120+**. It shows you how I go about creating an external aim assist. It's not the best method, but it works.

This one only contains aimassist. You can type the command `aimassist` to discover the many possible modifications.
I may add a __PearlAimbot__ in the future, depending on how I feel ;)

This project uses signatures and a detour/shellcode system. Several hooks are implemented in the game to retrieve the `LocalPlayer`, `HitResultActor`, and `Camera`. It supports virtually any type of detour automatically. You will need to implement the number of bytes to be moved manually if you want to add hooks, of course.

This project was created and designed by me. If you have any questions, you can contact me on Discord using my username: `Zwuiix`.
You can also join the [Kodiak](https://discord.gg/9kUthmEZhw) Discord server.
