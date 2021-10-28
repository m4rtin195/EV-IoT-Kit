---
title: "Communication technologies"
permalink: /docs/communication-technologies/
---

Another part of our system is the communication technology, using which the transfer of data between the vehicle and cloud is realized. 

There are many different requirements for the vehicle’s connectivity, resulting from the various functions that a modern vehicle should provide. Some of this requirements are hard to combine, so multiple technologies may be used. For example, the infotainment may need full-band connectivity to the internet during the ride; the OTA updates may need to transfer big amounts of data, ideally through Wi-Fi; and for the transmission of small pieces of telemetric or telematic data it is much appropriate to have a technology which have a great range, and not necessarily high transfer speeds or capacity.

The connectivity between vehicle and cloud can be realized in a various ways. Basically, we can devide communication technologies into the 3 categories:
1. **Local WLAN technologies.** These are grouped under 802.11 IEEE standards group and have a common marketing name *Wi-Fi*. The “ordinary” Wi-Fi which we know from our homes also belongs here, but there are many other technologies operating in various, mostly unlicensied bands, on smaller distances between tens of meters until very few kilometers. We can name 802.11ah and 802.11af standards, or 802.11bd designed especially for next generation vehicles communication. The big advantage is the big transfer capacity and speed, but there are no public operators of them. Good solution for own or partner’s charging-stations network and connectivity at home.

2. **Broadband mobile technologies**, such a GSM, LTE or 5G. These provides good transfer speeds and low latency, but they are hard to manage, especially when we consider the need for international operability. The roaming has to be dealt with, there is a need to implement SIM cards, contracts with local operators, and so on. Good solution for higher-class vehicles, where internet access can be shared also with onboard infotainment and passengers smart devices.

3. **LPWAN technologies**. This is the group of modern communication technologies designed for use in the area of IoT (Internet of Things) devices, which usually offers low to very-low bandwidth and transfer speeds, in favor of great coverage around base stations and very low energy consumption. In vehicles, we can reap these 

### LPWAN
means Low-Power Wide-Area-Network.

The last mentioned, LPWAN technologies were the main area of interest connectivity method for my project (diploma thesis). Since whole this area of technologies and market of them is relatively new, it is still being formed, individual technologies compete between each other, lot of them wane, and new one are emerging. If you are interested in this, in my diploma thesis there is a long chapter about this technologies and comparison of them (in Slovak only). 

In short, we know 3 approaches, how to reach mentioned key features of LPWAN, forming 3 categories of these technologies:

#### Ultra Narrowband
The signal is modulated into extremelly narrow frequency bandwidth. This achieves a low level of accumulated noise, thus high sensitivity during reception is possible, and thus a very high range can be achieved. The most known technology of this type is Sigfox, which we also used for this project. The problematic part is, that low bandwidth means low transfer speed, and low transfer speed means long transmission time. And low transmission time means high probability of collisions with broadcasting of other endpoints/technologies. If technology uses non-licenced and thus non-controlled frequency band, this can be a serious problem. Possible solution is to broadcast the same message more times, which increase the probability of sucessful delivery on the one hand, but overhelm the spectrum even more on the other hand.

#### Spread Spectrum
In this case, signal with useful information is modulated into, vice-versa, very wide frequency band. The resulting signal has the very low amplitude near the level of noise. To distinguish the signal from the noise in the receiver, some specific characteristic of the carrier signal has to be used. This principe makes the transfer really interference resistant from the other systems in the same band, naturaly secure (its very hard to eavesdrop the transmission) and thanks to wide bandwith, the transfer speed can be also quite high. The dark side is however the durability against other spread spectrum broadcastings. Because the used frequency bandwidth is so wide, it can easily overlaps with the spectrum of other broadcasting. To avoid this, more different spread factors can be used for separate broadcastings. But it creates a need to handshake endpoints with base station before useful broadcasting, there must be some controller to organize this, and things are getting complicated. In addition, different spreading factors means different range and speeds for different configurations. The representative technology of this category is LoRa.

#### Celluar
The previous explained technologies usually operates in so-called ISM, unlicenced bands, where more systems can operate on the same frequencies, and they do not synchronize each other in any way. Therefore, in different parts of the world, there are various regulations on fair using this free bands. 
On the other end, there are licenced bands, which are privilegedly allocated to operators by some regulation authority. This is common and necessary in well-known classic mobile networks, which are built on cells architecture. The cell is area covered by one base station, which control it and take care about synchronization and all the stuff to make transmissions safe and durable. Because this bands are heavily occupied just by mobile services and is already divided between operators, the LPWAN technogies which can be implemented between them has emerged. The examples are LTE-M and Nb-IoT, which can coexist in with classic mobile networks in the same bands or even cooperate with them. They use the same or similiar principes but are truncated of some features. The advantage is perfect robustness and reliability, the cons are higher costs, need for authorization in network (SIM cards), compelxity of system and so on.

As we can see, all of these approaches has some pros and cons. Therefore, it is very important to invest sufficient amount of time to consider what is needed for a specific project, which properties are necessary and what technology can bring us the best “price-performance” ratio.

Finally, we decided to use *Sigfox* technology as the main connectivity method between our simulator and the cloud backend. In the addition, we later also implemented the way to utilize a standard internet connection of simulator if it is available, and therefore a WLAN type connectivity from the vehicles’s point of way.

See more about Sigfox ~in separate post.~