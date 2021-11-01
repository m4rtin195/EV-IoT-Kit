---
title: "Communication Technologies"
permalink: /docs/communication-technologies/
---

Another part of our system is the communication technology, using which the transfer of data between <i>the vehicle</i> and <i>cloud</i> is realized. 

There are many different requirements for the vehicle’s connectivity, resulting from the various functions that a modern vehicle should provide. Some of these requirements are hard to combine, so multiple technologies may be used. For example, the infotainment may need full-band connectivity to the internet <u>during the ride</u>; the <abbr title="Over the Air">OTA</abbr> updates may need to transfer big amounts of data, ideally through Wi-Fi; and for the transmission of small pieces of telemetric or telematic data, it is much appropriate to have a technology which has a great range, but not necessarily high transfer speeds or capacity.

The connectivity between vehicle and cloud can be realized in various ways. Basically, we can divide communication technologies into the 3 categories:

1. **Local WLAN technologies.** These are grouped under 802.11 IEEE standards group and have a common marketing name *Wi-Fi*. The “ordinary” Wi-Fi which we know from our homes also belongs here, but there are many other technologies operating in various, mostly unlicensed bands, on smaller distances between tens of meters until very few kilometers. We can name `802.11ah` and `802.11af` standards, or `802.11bd` designed especially for the next-generation vehicles communication. The big advantage is the big transfer capacity and speed, but there are no public operators of them. They are a good solution for own or partner’s charging-stations network, and connectivity at home.

2. **Broadband mobile technologies**, such a `GSM`, `LTE`, or `5G`. They provide good transfer speeds and low latency, but they are hard to manage, especially when we consider the need for international operability. The roaming has to be dealt with, there is a need to implement SIM cards, contracts with local operators, and so on. A good solution for higher-class vehicles, where internet access can be shared also with onboard infotainment and passenger's smart devices.

3. **LPWAN technologies**. This is the group of modern communication technologies designed for use in the area of IoT (*Internet of Things*) devices, which usually offer low to very-low bandwidth and transfer speeds, in favor of great coverage around base stations and very low energy consumption. In vehicles, we can reap their benefits to have a reliable connectivity for transfering operational data.

<br>

### LPWAN
Means *Low-Power Wide-Area-Network*.

The last mentioned, LPWAN technologies, were the main area of interest connectivity method for my project. Since whole this area of technologies and market of them is relatively new, it is still being formed. Individual technologies compete with each other, a lot of them wane, and new ones are emerging. If you are interested in this, in the theory of my diploma thesis there is a long chapter about these technologies and a comparison of them (in Slovak only). 

In short, we know 3 approaches how to reach mentioned key features of LPWAN, forming 3 categories of these technologies:

<br>
<ul class="nav nav-tabs">
  <li class="active"><a href="#unb" data-toggle="tab">Ultra Narrowband</a></li>
  <li><a href="#ss" data-toggle="tab">Spread Spectrum</a></li>
  <li><a href="#cellular" data-toggle="tab">Cellular-based</a></li>
</ul>

<div id="myTabContent" class="tab-content">
  <div class="tab-pane fade active in" id="unb">
  	<div class="bs-component">
        <div class="well">
  			<h4>Ultra Narrowband</h4>
    		<p>The signal is modulated into an extremely <b>narrow frequency band</b>. This achieves a low level of accumulated noise, thus high sensitivity during the reception is possible, and thus a <i>very high range</i> can be achieved. <br>
			The most known technology of this type is <code>Sigfox</code>, which we also used for this project.</p>
			<p>The problematic part is, that low bandwidth means <i>low transfer speed</i>, and low transfer speed means long transmission time. And low transmission time means a high probability of collisions with the broadcasting of other endpoints/technologies. If technology uses a non-licensed (and thus non-controlled) frequency band, this can be a serious problem. A possible solution is to broadcast the same message more times, which increases the probability of successful delivery on the one hand, but overwhelms the spectrum even more on the other hand.</p>
    	</div>
    </div>
  </div>

  <div class="tab-pane fade" id="ss">
  	<div class="bs-component">
        <div class="well">
            <h4>Spread Spectrum</h4>
    		<p>In this case, a signal with useful information is modulated into, vice-versa, a <b>very wide frequency band</b>. The resulting signal has a very low amplitude near the level of noise. To distinguish the signal from the noise in the receiver, some specific characteristic of the carrier signal has to be used.</p>
			<p>This principle makes the transfer really <i>interference-resistant</i> from the other systems in the same band, <i>naturally secure</i> (it is very hard to eavesdrop on the transmission), and thanks to the wide bandwidth, the transfer <i>speed can be also quite high</i>. The dark side is however the durability against other spread spectrum broadcastings. Because the used frequency bandwidth is so wide, it can easily overlap with the spectrum of other broadcasting. To avoid this, more different spread factors can be used for separate broadcastings. But it creates a need to handshake endpoints with the base station before useful broadcasting, there must be some controller to organize this, and things are getting complicated. In addition, different spreading factors means different ranges and speeds for different configurations.</p> 
			<p>The representative technology of this category is <code>LoRa</code>.</p>
        </div>
    </div>	
  </div>

  <div class="tab-pane fade" id="cellular">
  	<div class="bs-component">
        <div class="well">
        	<h4>Cellular-based</h4>
            <p>The previously explained technologies usually operate in so-called ISM, unlicensed bands, where more systems can operate on the same frequencies, and they do not synchronize each other in any way. Therefore, in different parts of the world, there are various regulations on fair using these free bands.</p>
			<p>On the other end, there are <b>licensed bands</b>, which are privilegedly allocated to operators by some regulation authority. This is common and necessary in well-known classic mobile networks, which are built on cell-based architecture. The cell is the area covered by one base station, which controls it and takes care of synchronization and all the stuff to make transmissions safe and durable. Because these bands are heavily occupied by mobile services and are already divided between operators, the LPWAN technologies which can be implemented between them have emerged.</p>
			<p>The examples are <code>LTE-M</code> and <code>Nb-IoT</code>, which can coexist with classic mobile networks in the same bands or even cooperate with them. They use the same or similar principles, but are truncated of some features. The advantage is the <i>perfect robustness and reliability</i>, the cons are <i>higher costs</i>, the <i>need for authorization</i> into the network (SIM cards), complexity of the system, and so on.</p>
        </div>
    </div>
  </div>
</div>

<br>

As we can see, all of these approaches have some pros and cons. Therefore, it is very important to invest a sufficient amount of time to consider what is needed for a specific project - which properties are necessary and what technology can bring us the best “price/performance” ratio.

Finally, we decided to use **Sigfox** as the main connectivity method between our simulator and the cloud backend. In addition, later we also implemented the way to utilize a standard internet connection of simulator if it is available (more in [Simulator article](/docs/vehicle-simulator/)), and therefore a WLAN-type connectivity, from the vehicles point of view.

See more about Sigfox [in a separate post](/docs/sigfox/).