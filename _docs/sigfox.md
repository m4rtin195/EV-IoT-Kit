---
title: "Sigfox"
permalink: /docs/sigfox/
---

<div style="display:flex; flex-flow:row wrap; justify-content:center">
  <div style="flex-grow:1; flex-basis:60%">
    <p>Sigfox, sometimes called as a low-speed 0G network, is one of the most popular LPWAN technologies, used globally around the whole world for various M2M applications. It works on the Ultra-Narrowband principle to achieve probably the biggest range across all its competitors, in parallel with extremely low energy consumption.</p>
  </div>
  <div style="flex-grow:0; flex-basis:40%;">
  	<a href="https://www.sigfox.com/" target="_blank"><img src="{{ "/assets/img/docs/sigfox_logo.png" | relative_url }}" alt="sigfox logo" style="max-width:100%; padding-left:20px; padding-right:30px; padding-top:10px;"></a>
  </div>
</div>

It's a patented technology of the same-named **French company**, which also operates the main “hub”, cloud, where all the messages from all the endpoints end. If the previous sentence sounds weird to you, it is because the Sigfox network(s) works in a really unique way. Namely, in every country, only one operator can operate the Sigfox network, and subsequently, he forward all the received messages from all operated <abbr title="Base Transceiver Station">BTS</abbr>'s, through the secured **VPN connection**, to the main Sigfox cloud, called `Sigfox Backend`, which process them and forward them to their recipients. Thanks to this, Sigfox can be considered as the one global network without geographical borders. Also from the technical point of view, things are designed in the way, that devices can seamlessly move between more networks (countries borders), without need to deal with roaming or changing radio configurations[^1].

When it comes to availability, today Sigfox operates in **77 countries** of the world and covers **1,3 billion people**. The highest density of coverage is in Europe, where most of the countries have, or will have practically full coverage, except some mountainous areas, etc.

### The principle of operation
It is quite simple. The message is modulated into the **bandwith of only 100Hz**[^2] using the <abbr title="Differential Binary Phase-shift Keying">DBPSK</abbr> (for uplink) or <abbr title="Gaussian Frequency-shift Keying">GFSK</abbr> modulation (for downlink), and is blindly broadcasted into the ether, without any previous negotiation with the "network", or even detecting it. One or more base stations in the sorrounding can receive the signal, and after sucessful CRC check, it (/all of them) sends it to Sigfox cloud, as explained above. 

BTS’s do not provide any acknowledgement of reception back to the endpoint by default (downlink message can be used for it, but must be initiated by cloud, not BTS itself). In addition, **each message from endpoint is broadcasted 3 times** (default setting), with random time and frequency offsets, to increase probability that at least one broadcast will be received by some BTS (different signal propagation depending on frequency in the freq. domain, and collisions with any other broadcasts, in the time domain). 

### The tech details
The <b>maximum payload of one message is just 12 bytes</b>. The <b>transmission speed</b> of Sigfox is strictly set to <b>100bits/s</b>. While it looks enormously small and slow, it is enough to transfer the small amount of information between machines, once in a while. After all, this is the goal of this technology. Sigfox is mainly focused on upstream communication (from endpoints to server), but it supports also downstream messages (in smaller amount).
Whole transmission chain is secured by end-to-end <b>AES128 encryption</b>, the payload is not decrypted until the cloud.

Thanks to the fact, that energy is radiated in a narrow band, reach of the signal is much higher, compared to conventional technologies which uses bandwidth of hundreds of kHz. Actually, the whole small country can be covered by few base stations. 
Also, it brings very low power-requirements, so Sigfox is great for small battery powered devices, which can last to work for up to 10 years without the battery change.
The cons of Sigfox is the tight payload limit, <u>limited amount of messages per day</u> (it results from regulations of ISM bands and touches all the technologies working in unlicenced bands) and no acknowledgment of receipt.


Overall, Sigfox, along with its main competitor (LoRa technology) are considered as the leading LPWAN technologies in the unlicensed (ISM) frequency bands. 

### Why we chosed it
The reason why we chosed Sigfox as the used technology for our project is its simplicity, cheapness, reliability, available global network operated by trustworthy authority, and the seamless roaming possible around the whole globe. It means that the user can travel with his vehicle between the countries, and its functionality will be always available, without the need to set or configure something.

The topic of Sigfox is widely covered in my diploma thesis (Slovak only).
<br>
<br>

[^1]: To be exact, more regions with different radio configurations exists around the globe, and not all modems supports more, or all of them. But there are also those who do, and switching between them is automatic. Search for *Sigfox Monarch*.

[^2]: or 600Hz in some radio-configuration regions.