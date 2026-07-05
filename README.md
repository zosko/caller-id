# Caller ID

Caller ID module that can be used in various projects, providing reliable identification of incoming calls along with essential details such as phone numbers, timestamps, and optional contact name resolution. This module ([origin](https://github.com/dilshan/arduino-caller-id)) is designed and modified to be easily integrated into different systems.

## Scheme
![](images/scheme.png)

Board | Preview 
-----|-----
![](images/board.png)|![](images/preview.png)

## Package example

```
INCOMING CALL
MDMF PACKET
 MSG HEADER: 1
 MSG LEN: 8
 MSG END 07050400
Jul0504:00
 MSG HEADER: 2
 MSG LEN: 9
 MSG END 055857857
055857857
 MSG HEADER: 7
 MSG LEN: E
 MSG END Bosko Pet
Bosko Pet
MDMF PACKET END
````

## Sponsorship

![PCBWay_logo](images/pcbway_logo.png)

**This project is proudly sponsored by [PCBWay](https://pcbway.com).**

PCBWay specializes in manufacturing high-quality printed circuit boards (PCBs), making them accessible and affordable for both hobbyists and professionals.

Their range of services includes:

- PCB prototyping
- Assembly services
- Instant order quotations
- Expert verification processes
- A user-friendly, hassle-free ordering system

I'm sincerely grateful to PCBWay for their generous support in making this project possible.
