# The rules of durak on the stand: the scripted games of phase 1

Run on 2026-11-24 on the diag stand (DayZ 1.30.164289, the world of instance 83, CF, the core,
the MCP bridge, no client), with the repository at commit `f576a6a`. Every row is one
`world_exec` of the verb `oz_games`; the plan is
[2026-11-24-phase-1-plan.md](../../2026-11-24-phase-1-plan.md), Task 5, where each row
also says what it shows. The answers below are copied from the server's own log of the
boot, `script_2026-11-24_01-18-37.log`.

## Table 1: a game for three, podkidnoy

**1.1** `{"op": "begin", "seats": "0,1,2", "deck": "6s 6h 7s Th Jd 7c 8s 8h Ts Td Qh Kc 6d 7h 9s 8d Js Ah Qs 9h 7d 8c Ks Jh 9d 9c As Kh Qd Tc Kd Ad Jc Qc Ac 6c"}` — as expected

```text
durak podkidnoy trump=6c stock=18 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=0
```

**1.2** `{"op": "play", "moves": "2 a:6d; 1 a:8s; 0 a:Ac; 0 pass; 1 take; 0 x:1; 7 a:6s"}` — as expected

```text
-STR_OZG_DURAK_NO_TURN -STR_OZG_DURAK_NO_DEFENDER -STR_OZG_NO_CARD -STR_OZG_DURAK_NO_EMPTY -STR_OZG_DURAK_NO_EMPTY -STR_OZG_NO_MOVE -STR_OZG_NO_SEAT | durak podkidnoy trump=6c stock=18 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=0
```

**1.3** `{"op": "can", "seat": "0"}` — as expected

```text
seat 0 waits=yes skippable=no can: a:6s a:6h a:7s a:Th a:Jd a:7c
```

**1.4** `{"op": "play", "moves": "0 a:6s; 0 a:6h; 2 a:9s; 2 a:6d"}` — as expected

```text
+ + -STR_OZG_DURAK_NO_RANK + | durak podkidnoy trump=6c stock=18 dis=0 table=6s/-,6h/-,6d/- op=0 def=1 lim=5 take=0 first=1 hands=0:4,1:6,2:5 waits=1
```

**1.5** `{"op": "can", "seat": "1"}` — as expected

```text
seat 1 waits=yes skippable=no can: d:8s:1 d:Ts:1 d:Kc:1 d:8h:2 d:Qh:2 d:Kc:2 d:Td:3 d:Kc:3 take
```

**1.6** `{"op": "play", "moves": "1 t:8s; 1 d:8s:1; 1 d:8h:1; 1 d:Ts:2; 0 d:7s:2; 1 d:8h:2"}` — as expected

```text
-STR_OZG_DURAK_NO_TRANSFER_OFF + -STR_OZG_DURAK_NO_TARGET -STR_OZG_DURAK_NO_BEAT -STR_OZG_DURAK_NO_NOT_DEFENDER + | durak podkidnoy trump=6c stock=18 dis=0 table=6s/8s,6h/8h,6d/- op=0 def=1 lim=5 take=0 first=1 hands=0:4,1:4,2:5 waits=1
```

**1.7** `{"op": "play", "moves": "2 a:8d; 1 d:Td:3; 0 a:Th; 0 a:7s"}` — as expected

```text
+ + + -STR_OZG_DURAK_NO_LIMIT | durak podkidnoy trump=6c stock=18 dis=0 table=6s/8s,6h/8h,6d/Td,8d/-,Th/- op=0 def=1 lim=5 take=0 first=1 hands=0:3,1:3,2:4 waits=1
```

**1.8** `{"op": "play", "moves": "1 d:Qh:5; 1 d:Kc:4"}` — as expected

```text
+ + | durak podkidnoy trump=6c stock=8 dis=10 table=- op=1 def=2 lim=6 take=0 first=0 hands=0:6,1:6,2:6 waits=1
```

**1.9** `{"op": "all"}` — as expected

```text
0[7s Jd 7c Qs 9h 7d] 1[Ts Jh 9d 9c As Kh] 2[7h 9s Js Ah 8c Ks] table[-] stock[Qd Tc Kd Ad Jc Qc Ac 6c]
```

**1.10** `{"op": "play", "moves": "1 a:9d; 2 take"}` — as expected

```text
+ + | durak podkidnoy trump=6c stock=8 dis=10 table=9d/- op=1 def=2 lim=6 take=1 first=0 hands=0:6,1:5,2:6 waits=0,1
```

**1.11** `{"op": "play", "moves": "2 d:8c:1; 2 take; 1 pass; 1 pass"}` — as expected

```text
-STR_OZG_DURAK_NO_TAKING -STR_OZG_DURAK_NO_TAKING + -STR_OZG_DURAK_NO_PASS | durak podkidnoy trump=6c stock=8 dis=10 table=9d/- op=1 def=2 lim=6 take=1 first=0 hands=0:6,1:5p,2:6 waits=0
```

**1.12** `{"op": "act", "seat": "0", "move": "a:9h"}` — as expected

```text
ok | durak podkidnoy trump=6c stock=8 dis=10 table=9d/-,9h/- op=1 def=2 lim=6 take=1 first=0 hands=0:5,1:5,2:6 waits=1
```

**1.13** `{"op": "act", "seat": "1", "move": "pass"}` — as expected

```text
ok | durak podkidnoy trump=6c stock=6 dis=10 table=- op=0 def=1 lim=6 take=0 first=0 hands=0:6,1:6,2:8 waits=0
```

**1.14** `{"op": "play", "moves": "0 a:7s; 0 a:7d; 0 a:7c; 2 a:7h; 1 d:As:3; 1 d:Ts:1; 1 d:Qd:2; 1 d:9c:3; 1 d:Jh:4"}` — as expected

```text
+ + + + -STR_OZG_DURAK_NO_BEAT + + + + | durak podkidnoy trump=6c stock=6 dis=10 table=7s/Ts,7d/Qd,7c/9c,7h/Jh op=0 def=1 lim=6 take=0 first=0 hands=0:3,1:2,2:7 waits=0,2
```

**1.15** `{"op": "play", "moves": "0 a:Jd; 2 a:9s; 2 a:9d"}` — as expected

```text
+ + -STR_OZG_DURAK_NO_LIMIT | durak podkidnoy trump=6c stock=6 dis=10 table=7s/Ts,7d/Qd,7c/9c,7h/Jh,Jd/-,9s/- op=0 def=1 lim=6 take=0 first=0 hands=0:2,1:2,2:6 waits=1
```

**1.16** `{"op": "can", "seat": "1"}` — as expected

```text
seat 1 waits=yes skippable=no can: d:As:6 take
```

**1.17** `{"op": "act", "seat": "1", "move": "take"}` — as expected

```text
ok | durak podkidnoy trump=6c stock=2 dis=10 table=- op=2 def=0 lim=6 take=0 first=0 hands=0:6,1:12,2:6 waits=2
```

**1.18** `{"op": "all"}` — as expected

```text
0[Qs Tc Kd Ad Jc Qc] 1[As Kh 7s Ts 7d Qd 7c 9c 7h Jh Jd 9s] 2[Js Ah 8c Ks 9d 9h] table[-] stock[Ac 6c]
```

## Table 2: six seats, and how a game ends

**2.1** `{"op": "begin", "seats": "0,1,2,3,4,5", "deck": "6s 6d 6c 9s 9d 9c 7s 7d 7c 8s 8d 8c Ts Js Qs Ks As Td Jd Qd Kd Ad Tc Jc Qc Kc Ac 6h 7h 8h 9h Th Jh Qh Kh Ah"}` — as expected

```text
durak podkidnoy trump=Ah stock=0 dis=0 table=- op=4 def=5 lim=5 take=0 first=1 hands=0:6,1:6,2:6,3:6,4:6,5:6 waits=4
```

**2.2** `{"op": "all"}` — as expected

```text
0[6s 6d 6c 9s 9d 9c] 1[7s 7d 7c 8s 8d 8c] 2[Ts Js Qs Ks As Td] 3[Jd Qd Kd Ad Tc Jc] 4[Qc Kc Ac 6h 7h 8h] 5[9h Th Jh Qh Kh Ah] table[-] stock[]
```

**2.3** `{"op": "drop", "seat": "2"}` — as expected

```text
dropped 2 | durak podkidnoy trump=Ah stock=0 dis=6 table=- op=3 def=4 lim=5 take=0 first=1 hands=0:6,1:6,2:0d,3:6,4:6,5:6 waits=3
```

**2.4** `{"op": "drop", "seat": "3"}` — as expected

```text
dropped 3 | durak podkidnoy trump=Ah stock=0 dis=12 table=- op=4 def=5 lim=5 take=0 first=1 hands=0:6,1:6,2:0d,3:0d,4:6,5:6 waits=4
```

**2.5** `{"op": "drop", "seat": "4"}` — as expected

```text
dropped 4 | durak podkidnoy trump=Ah stock=0 dis=18 table=- op=5 def=0 lim=5 take=0 first=1 hands=0:6,1:6,2:0d,3:0d,4:0d,5:6 waits=5
```

**2.6** `{"op": "drop", "seat": "5"}` — as expected

```text
dropped 5 | durak podkidnoy trump=Ah stock=0 dis=24 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:0d,3:0d,4:0d,5:0d waits=0
```

**2.7** `{"op": "drop", "seat": "5"}` — as expected

```text
dropped 5 | durak podkidnoy trump=Ah stock=0 dis=24 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:0d,3:0d,4:0d,5:0d waits=0
```

**2.8** `{"op": "play", "moves": "0 a:6s; 0 a:6d; 0 a:6c; 1 d:7s:1; 1 d:7d:2; 1 d:7c:3"}` — as expected

```text
+ + + + + + | durak podkidnoy trump=Ah stock=0 dis=30 table=- op=1 def=0 lim=3 take=0 first=0 hands=0:3,1:3,2:0d,3:0d,4:0d,5:0d waits=1
```

**2.9** `{"op": "play", "moves": "1 a:8s; 1 a:8d; 1 a:8c"}` — as expected

```text
+ + + | durak podkidnoy trump=Ah stock=0 dis=30 table=8s/-,8d/-,8c/- op=1 def=0 lim=3 take=0 first=0 hands=0:3,1:0,2:0d,3:0d,4:0d,5:0d waits=0
```

**2.10** `{"op": "can", "seat": "0"}` — as expected

```text
seat 0 waits=yes skippable=no can: d:9s:1 d:9d:2 d:9c:3 take
```

**2.11** `{"op": "save"}` — as expected

```text
saved 666 B sound=yes -> games_save.json
```

**2.12** `{"op": "play", "moves": "0 d:9s:1; 0 d:9d:2; 0 d:9c:3; 0 a:6s"}` — as expected

```text
+ + + -STR_OZG_NO_OVER | durak podkidnoy over tie trump=Ah stock=0 dis=36 hands=0:0x,1:0x,2:0d,3:0d,4:0d,5:0d
```

**2.13** `{"op": "outcome"}` — as expected

```text
outcome: durak=-1
```

**2.14** `{"op": "load"}` — as expected

```text
loaded | durak podkidnoy trump=Ah stock=0 dis=30 table=8s/-,8d/-,8c/- op=1 def=0 lim=3 take=0 first=0 hands=0:3,1:0,2:0d,3:0d,4:0d,5:0d waits=0
```

**2.15** `{"op": "act", "seat": "0", "move": "take"}` — as expected

```text
ok | durak podkidnoy over durak=0 trump=Ah stock=0 dis=30 hands=0:6,1:0x,2:0d,3:0d,4:0d,5:0d
```

**2.16** `{"op": "outcome"}` — as expected

```text
outcome: durak=0
```

**2.17** `{"op": "load"}` — as expected

```text
loaded | durak podkidnoy trump=Ah stock=0 dis=30 table=8s/-,8d/-,8c/- op=1 def=0 lim=3 take=0 first=0 hands=0:3,1:0,2:0d,3:0d,4:0d,5:0d waits=0
```

**2.18** `{"op": "drop", "seat": "1"}` — as expected

```text
dropped 1 | durak podkidnoy over nodurak trump=Ah stock=0 dis=33 hands=0:3,1:0d,2:0d,3:0d,4:0d,5:0d
```

**2.19** `{"op": "outcome"}` — as expected

```text
outcome: durak=-1
```

## Table 3: a seat dropped in the middle of a bout

**3.1** `{"op": "begin", "seats": "0,1,2,3", "deck": "6c 7s 8s 9s Ts Js 7h 8h 9h Th Jh Qh 8d 9d Td Jd Kh Ah 6s 6h 6d Qs Ks As"}` — as expected

```text
durak podkidnoy trump=Ac stock=12 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6,3:6 waits=0
```

**3.2** `{"op": "act", "seat": "0", "move": "a:7s"}` — as expected

```text
ok | durak podkidnoy trump=Ac stock=12 dis=0 table=7s/- op=0 def=1 lim=5 take=0 first=1 hands=0:5,1:6,2:6,3:6 waits=1
```

**3.3** `{"op": "drop", "seat": "3"}` — as expected

```text
dropped 3 | durak podkidnoy trump=Ac stock=11 dis=7 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6,3:0d waits=0
```

**3.4** `{"op": "play", "moves": "0 a:8s; 1 take"}` — as expected

```text
+ + | durak podkidnoy trump=Ac stock=11 dis=7 table=8s/- op=0 def=1 lim=5 take=1 first=1 hands=0:5,1:6,2:6,3:0d waits=2
```

**3.5** `{"op": "drop", "seat": "1"}` — as expected

```text
dropped 1 | durak podkidnoy trump=Ac stock=10 dis=14 table=- op=2 def=0 lim=5 take=0 first=1 hands=0:6,1:0d,2:6,3:0d waits=2
```

**3.6** `{"op": "play", "moves": "2 a:8d; 0 d:6c:1"}` — as expected

```text
+ + | durak podkidnoy trump=Ac stock=8 dis=16 table=- op=0 def=2 lim=6 take=0 first=0 hands=0:6,1:0d,2:6,3:0d waits=0
```

**3.7** `{"op": "all"}` — as expected

```text
0[9s Ts Js 7d Qd Ad] 1[] 2[9d Td Jd Kh Ah Kd] 3[] table[-] stock[7c 8c 9c Tc Jc Qc Kc Ac]
```

**3.8** `{"op": "act", "seat": "0", "move": "a:9s"}` — as expected

```text
ok | durak podkidnoy trump=Ac stock=8 dis=16 table=9s/- op=0 def=2 lim=6 take=0 first=0 hands=0:5,1:0d,2:6,3:0d waits=2
```

**3.9** `{"op": "drop", "seat": "0"}` — as expected

```text
dropped 0 | durak podkidnoy over nodurak trump=Ac stock=8 dis=22 hands=0:0d,1:0d,2:6,3:0d
```

## Table 4: perevodnoy

**4.1** `{"op": "begin", "seats": "0,1,2", "opt": "variant=perevodnoy", "deck": "6c 7s 8s 9s Ts Js 7h 8h 9h Th Jh Qh 7d 8d 9d Td Jd Qd"}` — as expected

```text
durak perevodnoy trump=Ac stock=18 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=0
```

**4.2** `{"op": "act", "seat": "0", "move": "a:7s"}` — as expected

```text
ok | durak perevodnoy trump=Ac stock=18 dis=0 table=7s/- op=0 def=1 lim=5 take=0 first=1 hands=0:5,1:6,2:6 waits=1
```

**4.3** `{"op": "can", "seat": "1"}` — as expected

```text
seat 1 waits=yes skippable=no can: t:7h take
```

**4.4** `{"op": "play", "moves": "1 t:8h; 2 t:7d; 1 t:7h"}` — as expected

```text
-STR_OZG_DURAK_NO_TRANSFER_RANK -STR_OZG_DURAK_NO_NOT_DEFENDER + | durak perevodnoy trump=Ac stock=18 dis=0 table=7s/-,7h/- op=0 def=2 lim=5 take=0 first=1 hands=0:5,1:5,2:6 waits=2
```

**4.5** `{"op": "act", "seat": "2", "move": "t:7d"}` — as expected

```text
ok | durak perevodnoy trump=Ac stock=18 dis=0 table=7s/-,7h/-,7d/- op=0 def=0 lim=5 take=0 first=1 hands=0:5,1:5,2:5 waits=0
```

**4.6** `{"op": "can", "seat": "0"}` — as expected

```text
seat 0 waits=yes skippable=no can: d:6c:1 d:8s:1 d:9s:1 d:Ts:1 d:Js:1 d:6c:2 d:6c:3 take
```

**4.7** `{"op": "play", "moves": "0 d:8s:1; 0 t:6c; 0 take"}` — as expected

```text
+ -STR_OZG_DURAK_NO_TRANSFER_LATE + | durak perevodnoy trump=Ac stock=18 dis=0 table=7s/8s,7h/-,7d/- op=0 def=0 lim=5 take=1 first=1 hands=0:4,1:5,2:5 waits=1,2
```

**4.8** `{"op": "play", "moves": "1 a:8h; 2 a:8d"}` — as expected

```text
+ + | durak perevodnoy trump=Ac stock=14 dis=0 table=- op=1 def=2 lim=6 take=0 first=0 hands=0:10,1:6,2:6 waits=1
```

**4.9** `{"op": "all"}` — as expected

```text
0[6c 9s Ts Js 7s 8s 7h 7d 8h 8d] 1[9h Th Jh Qh 6s Qs] 2[9d Td Jd Qd Ks As] table[-] stock[6h Kh Ah 6d Kd Ad 7c 8c 9c Tc Jc Qc Kc Ac]
```

**4.10** `{"op": "begin", "seats": "0,1", "opt": "variant=perevodnoy", "deck": "6c 7s 7h 7d 8s 9s 7c 8h 9h Th Jh Qh"}` — as expected

```text
durak perevodnoy trump=Ac stock=24 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6 waits=0
```

**4.11** `{"op": "play", "moves": "0 a:7s; 0 a:7h; 0 a:7d; 1 t:7c"}` — as expected

```text
+ + + -STR_OZG_DURAK_NO_TRANSFER_FEW | durak perevodnoy trump=Ac stock=24 dis=0 table=7s/-,7h/-,7d/- op=0 def=1 lim=5 take=0 first=1 hands=0:3,1:6 waits=1
```

**4.12** `{"op": "can", "seat": "1"}` — as expected

```text
seat 1 waits=yes skippable=no can: d:7c:1 d:7c:2 d:8h:2 d:9h:2 d:Th:2 d:Jh:2 d:Qh:2 d:7c:3 take
```

**4.13** `{"op": "begin", "seats": "0,1", "opt": "variant=perevodnoy", "deck": "6c 7s 7h 7d 8s 9s 7c 8h 9h Th Jh Qh"}` — as expected

```text
durak perevodnoy trump=Ac stock=24 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6 waits=0
```

**4.14** `{"op": "play", "moves": "0 a:7s; 1 t:7c; 0 t:7h"}` — as expected

```text
+ + + | durak perevodnoy trump=Ac stock=24 dis=0 table=7s/-,7c/-,7h/- op=0 def=1 lim=5 take=0 first=1 hands=0:4,1:5 waits=1
```

**4.15** `{"op": "can", "seat": "1"}` — as expected

```text
seat 1 waits=yes skippable=no can: d:8h:3 d:9h:3 d:Th:3 d:Jh:3 d:Qh:3 take
```

**4.16** `{"op": "play", "moves": "1 take; 0 a:7d"}` — as expected

```text
+ + | durak perevodnoy trump=Ac stock=21 dis=0 table=- op=0 def=1 lim=6 take=0 first=0 hands=0:6,1:9 waits=0
```

**4.17** `{"op": "all"}` — as expected

```text
0[6c 8s 9s 6s Ts Js] 1[8h 9h Th Jh Qh 7s 7c 7h 7d] table[-] stock[Qs Ks As 6h Kh Ah 6d 8d 9d Td Jd Qd Kd Ad 8c 9c Tc Jc Qc Kc Ac]
```

## Table 5: a player who is away, and one who comes back

**5.1** `{"op": "begin", "seats": "0,1,2", "deck": "6s 6h 7s Th Jd 7c 8s 8h Ts Td Qh Kc 6d 7h 9s 8d Js Ah Qs 9h 7d 8c Ks Jh 9d 9c As Kh Qd Tc Kd Ad Jc Qc Ac 6c"}` — as expected

```text
durak podkidnoy trump=6c stock=18 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=0
```

**5.2** `{"op": "away", "seats": "2"}` — as expected

```text
away: 2 | durak podkidnoy trump=6c stock=18 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=0
```

**5.3** `{"op": "play", "moves": "0 a:6s; 1 d:8s:1"}` — as expected

```text
+ + skipped=1 | durak podkidnoy trump=6c stock=18 dis=0 table=6s/8s op=0 def=1 lim=5 take=0 first=1 hands=0:5,1:5,2:6p waits=0
```

**5.4** `{"op": "can", "seat": "2"}` — as expected

```text
seat 2 waits=no skippable=no can: a:6d a:8d
```

**5.5** `{"op": "play", "moves": "0 a:6h; 1 d:8h:2"}` — as expected

```text
+ + skipped=1 | durak podkidnoy trump=6c stock=14 dis=4 table=- op=1 def=2 lim=6 take=0 first=0 hands=0:6,1:6,2:6 waits=1
```

**5.6** `{"op": "act", "seat": "1", "move": "a:Ts"}` — as expected

```text
ok | durak podkidnoy trump=6c stock=14 dis=4 table=Ts/- op=1 def=2 lim=6 take=0 first=0 hands=0:6,1:5,2:6 waits=2
```

**5.7** `{"op": "can", "seat": "2"}` — as expected

```text
seat 2 waits=yes skippable=no can: d:Js:1 take
```

**5.8** `{"op": "away", "seats": ""}` — as expected

```text
away: - | durak podkidnoy trump=6c stock=14 dis=4 table=Ts/- op=1 def=2 lim=6 take=0 first=0 hands=0:6,1:5,2:6 waits=2
```

**5.9** `{"op": "act", "seat": "2", "move": "d:Js:1"}` — as expected

```text
ok | durak podkidnoy trump=6c stock=14 dis=4 table=Ts/Js op=1 def=2 lim=6 take=0 first=0 hands=0:6,1:5,2:5 waits=0,1
```

**5.10** `{"op": "away", "seats": "0,1"}` — as expected

```text
away: 0,1 skipped=2 | durak podkidnoy trump=6c stock=12 dis=6 table=- op=2 def=0 lim=6 take=0 first=0 hands=0:6,1:6,2:6 waits=2
```

**5.11** `{"op": "away", "seats": "2"}` — as expected

```text
away: 2 | durak podkidnoy trump=6c stock=12 dis=6 table=- op=2 def=0 lim=6 take=0 first=0 hands=0:6,1:6,2:6 waits=2
```

**5.12** `{"op": "can", "seat": "2"}` — as expected

```text
seat 2 waits=yes skippable=no can: a:6d a:7h a:9s a:8d a:Ah a:Jh
```

## Table 6: what a client is told, and what is saved

**6.1** `{"op": "away", "seats": ""}` — as expected

```text
away: - | durak podkidnoy trump=6c stock=12 dis=6 table=- op=2 def=0 lim=6 take=0 first=0 hands=0:6,1:6,2:6 waits=2
```

**6.2** `{"op": "act", "seat": "2", "move": "a:6d"}` — as expected

```text
ok | durak podkidnoy trump=6c stock=12 dis=6 table=6d/- op=2 def=0 lim=6 take=0 first=0 hands=0:6,1:6,2:5 waits=0
```

**6.3** `{"op": "view", "seat": "0"}` — as expected

```text
view: public 393 B, seat 0 77 B -> games_view.txt
```

`games_view.txt`, two lines: what everybody is told, then seat 0's own half. As the plan prints them.

```text
{"GameId":"durak","VariantId":"podkidnoy","TrumpCard":"6c","StockCount":12,"DiscardCount":6,"AttackCards":["6d"],"DefenceCards":[""],"OpenerSeat":2,"DefenderSeat":0,"AttackLimit":6,"Taking":0,"SeatList":[{"SeatNo":0,"CardCount":6,"Gone":0,"Passed":0},{"SeatNo":1,"CardCount":6,"Gone":0,"Passed":0},{"SeatNo":2,"CardCount":5,"Gone":0,"Passed":0}],"Awaited":[0],"Ended":0,"DurakSeat":-1,"Tie":0}
{"Held":["7s","Th","Jd","7c","Qs","9h"],"CanPlay":["d:Jd:1","d:7c:1","take"]}
```

**6.4** `{"op": "save"}` — as expected

```text
saved 528 B sound=yes -> games_save.json
```

`games_save.json`, the whole state. As the plan prints it.

```text
{"Version":1,"Transfer":0,"SeatList":[{"SeatNo":0,"Held":["7s","Th","Jd","7c","Qs","9h"],"Gone":0,"Passed":0},{"SeatNo":1,"Held":["Td","Qh","Kc","7d","8c","Ks"],"Gone":0,"Passed":0},{"SeatNo":2,"Held":["7h","9s","8d","Ah","Jh"],"Gone":0,"Passed":0}],"StockCards":["9d","9c","As","Kh","Qd","Tc","Kd","Ad","Jc","Qc","Ac","6c"],"TrumpCard":"6c","DiscardCards":["6s","8s","6h","8h","Ts","Js"],"AttackCards":["6d"],"DefenceCards":[""],"Opener":2,"Defender":0,"AttackLimit":6,"Taking":0,"FirstBout":0,"Ended":0,"DurakSeat":-1,"Tie":0}
```

**6.5** `{"op": "act", "seat": "0", "move": "take"}` — as expected

```text
ok | durak podkidnoy trump=6c stock=11 dis=6 table=- op=1 def=2 lim=6 take=0 first=0 hands=0:7,1:6,2:6 waits=1
```

**6.6** `{"op": "load"}` — as expected

```text
loaded | durak podkidnoy trump=6c stock=12 dis=6 table=6d/- op=2 def=0 lim=6 take=0 first=0 hands=0:6,1:6,2:5 waits=0
```

**6.7** `{"op": "load", "swap": "6d>7h"}` — as expected

```text
not loaded, the game on the bench stays | durak podkidnoy trump=6c stock=12 dis=6 table=6d/- op=2 def=0 lim=6 take=0 first=0 hands=0:6,1:6,2:5 waits=0
```

**6.8** `{"op": "load", "swap": "Version>Vers"}` — as expected

```text
not loaded, the game on the bench stays | durak podkidnoy trump=6c stock=12 dis=6 table=6d/- op=2 def=0 lim=6 take=0 first=0 hands=0:6,1:6,2:5 waits=0
```

## Table 7: who opens a game

**7.1** `{"op": "begin", "seats": "0,1,2", "hint": "host=0;durak=2", "deck": "6s 6h 7s Th Jd 7c 8s 8h Ts Td Qh Kc 6d 7h 9s 8d Js Ah Qs 9h 7d 8c Ks Jh 9d 9c As Kh Qd Tc Kd Ad Jc Qc Ac 6c"}` — as expected

```text
durak podkidnoy trump=6c stock=18 dis=0 table=- op=1 def=2 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=1
```

**7.2** `{"op": "begin", "seats": "0,1,2", "hint": "host=0;durak=0", "deck": "6s 6h 7s Th Jd 7c 8s 8h Ts Td Qh Kc 6d 7h 9s 8d Js Ah Qs 9h 7d 8c Ks Jh 9d 9c As Kh Qd Tc Kd Ad Jc Qc Ac 6c"}` — as expected

```text
durak podkidnoy trump=6c stock=18 dis=0 table=- op=2 def=0 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=2
```

**7.3** `{"op": "begin", "seats": "0,1,2", "hint": "host=1;durak=5", "deck": "6s 6h 7s Th Jd 7c 8s 8h Ts Td Qh Kc 6d 7h 9s 8d Js Ah Qs 9h 7d 8c Ks Jh 9d 9c As Kh Qd Tc Kd Ad Jc Qc Ac 6c"}` — as expected

```text
durak podkidnoy trump=6c stock=18 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=0
```

**7.4** `{"op": "begin", "seats": "0,1,2", "hint": "host=0;durak=-1", "deck": "6s 6h 7s Th Jd 7c 8s 8h Ts Td Qh Kc 6d 7h 9s 8d Js Ah Qs 9h 7d 8c Ks Jh 9d 9c As Kh Qd Tc Kd Ad Jc Qc Ac 6c"}` — as expected

```text
durak podkidnoy trump=6c stock=18 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6,2:6 waits=0
```

**7.5** `{"op": "begin", "seats": "0,1", "hint": "host=0", "deck": "6s 7s 8s 9s Ts Js 6h 7h 8h 9h Th Jh"}` — as expected

```text
durak podkidnoy trump=Ac stock=24 dis=0 table=- op=1 def=0 lim=5 take=0 first=1 hands=0:6,1:6 waits=1
```

**7.6** `{"op": "begin", "seats": "0,1", "hint": "host=1", "deck": "6s 7s 8s 9s Ts Js 6h 7h 8h 9h Th Jh"}` — as expected

```text
durak podkidnoy trump=Ac stock=24 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6 waits=0
```

**7.7** `{"op": "begin", "seats": "0,1", "deck": "6s 7s 8s 9s Ts Js 6h 7h 8h 9h Th Jh"}` — as expected

```text
durak podkidnoy trump=Ac stock=24 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6 waits=0
```

**7.8** `{"op": "begin", "seats": "2,5", "hint": "host=3", "deck": "6s 7s 8s 9s Ts Js 6h 7h 8h 9h Th Jh"}` — as expected

```text
durak podkidnoy trump=Ac stock=24 dis=0 table=- op=5 def=2 lim=5 take=0 first=1 hands=2:6,5:6 waits=5
```

## Table 8: deals refused, and a deal that repeats

**8.1** `{"op": "begin", "seats": "0"}` — as expected

```text
no STR_OZG_NO_PLAYERS
```

**8.2** `{"op": "begin", "seats": "0,1,2,3,4,5,6"}` — as expected

```text
no STR_OZG_NO_PLAYERS
```

**8.3** `{"op": "begin", "game": "poker"}` — as expected (the command's status is `failed`, as it should be)

```text
no game 'poker' is registered; known: durak
```

**8.4** `{"op": "brief"}` — as expected

```text
durak podkidnoy trump=Ac stock=24 dis=0 table=- op=5 def=2 lim=5 take=0 first=1 hands=2:6,5:6 waits=5
```

**8.5** `{"op": "begin", "seats": "0,1", "seed": "7"}` — as expected

```text
durak podkidnoy trump=Jd stock=24 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6 waits=0
```

**8.6** `{"op": "all"}` — as expected

```text
0[8c 7s 8s Ks Ah 9d] 1[Ac Js Kh Jc Qc 9h] table[-] stock[Ad 9c Tc 8d As Ts Qh 9s 6h Td 7d Kc 8h 6c Kd 7h 6s 7c Jh Th 6d Qs Qd Jd]
```

**8.7** `{"op": "begin", "seats": "0,1", "seed": "7"}` — as expected

```text
durak podkidnoy trump=Jd stock=24 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6 waits=0
```

`log_verdict(source="server")` after these 98 rows: **pass**, no warning and no error; counters `games=1 problems=0`.

## Table 9: a save that is not JSON (after the verdict)

**9.1** `{"op": "save"}` — as expected

```text
saved 486 B sound=yes -> games_save.json
```

**9.2** `{"op": "load", "swap": "[>{"}` — as expected

```text
not loaded, the game on the bench stays | durak podkidnoy trump=Jd stock=24 dis=0 table=- op=0 def=1 lim=5 take=0 first=1 hands=0:6,1:6 waits=0
```

`log_verdict(source="server")` after rows 9.1 and 9.2: **fail**, with exactly one error, the engine's own, as the plan expects:

```text
SCRIPT    (E): JSON ERROR JM/CF/Scripts/3_Game/communityframework/game/dayzgame.c:46
```

## What the run shows

One line for each check of design §15 item 1, with the rows that show it:

| check | rows |
|---|---|
| allowed and refused moves | 1.2, 1.4, 1.6, 1.11, 1.14, 2.12 |
| the limit of five in a first bout, of six later, of the defender's hand | 1.7; 1.15; 2.8 and 2.9 |
| throwing in, by the opener and by a third seat | 1.4, 1.7, 1.14, 1.15 |
| taking, with cards thrown after | 1.10 to 1.13, 1.17, 4.7 and 4.8, 4.16 |
| moving the attack on, and when it may not be | 4.3 to 4.7, 4.11 to 4.14, 1.6 |
| the order of drawing | 1.9, 1.18, 4.9, 4.17 |
| the next attacker: after a defence, after a take | 1.8; 1.13, 1.17, 4.8 |
| the end: a durak, a tie, a game with no durak | 2.15; 2.12; 2.18, 3.9 |
| a seat dropped: at an empty table, with a card unbeaten, the defender after "take", the opener, a seat that has shed its cards | 2.3 to 2.6; 3.3; 3.5; 3.9; 2.18 |
| a seat whose player is away, and one that is taken up again | 5.3 to 5.12 |
| who opens | 1.1, 7.1 to 7.8 |
| what a client is told and what is saved | 6.3, 6.4, 6.6 to 6.8, 9.2 |

Nothing differed from the plan: all 100 answers came as it prints them, to the letter. The comparison was made by a script, between the plan's text and the server's log of the boot.

## Games played by chance

Task 6 of the plan, run on 2026-11-24 on the code of the commit that carries this section,
from the server's log `script_2026-11-24_01-28-56.log`. Each row is the op `fuzz` of `oz_games`: whole games
played by moves the game itself offers, with the state checked after every move and a save
loaded back every tenth. The last number of an answer is milliseconds.

**1** `{"op": "fuzz", "games": "300", "seed": "11"}` — podkidnoy, two to six seats by chance: as expected

```text
fuzz: 300 games, 52915 moves, longest 411, drops 0, skipped 0, with a loser 300, without 0, 0 defects, 11351 ms
```

**2** `{"op": "fuzz", "games": "300", "seed": "12", "opt": "variant=perevodnoy"}` — perevodnoy: as expected

```text
fuzz: 300 games, 54181 moves, longest 537, drops 0, skipped 0, with a loser 300, without 0, 0 defects, 11865 ms
```

**3** `{"op": "fuzz", "games": "300", "seed": "13", "opt": "variant=perevodnoy", "drop": "5", "away": "50"}` — seats dropped and players away: as expected

```text
fuzz: 300 games, 41866 moves, longest 346, drops 181, skipped 1485, with a loser 254, without 46, 0 defects, 8535 ms
```

**4** `{"op": "fuzz", "games": "300", "seed": "14", "players": "2", "opt": "variant=perevodnoy"}` — two seats: as expected

```text
fuzz: 300 games, 30524 moves, longest 180, drops 0, skipped 0, with a loser 300, without 0, 0 defects, 4150 ms
```

**5** `{"op": "fuzz", "games": "200", "seed": "15", "players": "6", "away": "100"}` — six seats and no stock: as expected

```text
fuzz: 200 games, 37703 moves, longest 396, drops 0, skipped 1212, with a loser 197, without 3, 0 defects, 8658 ms
```

**6** `{"op": "fuzz", "games": "200", "seed": "0", "drop": "20", "away": "30"}` — the real chance, asked twice: no defect either time, and the numbers differ, as they must

```text
fuzz: 200 games, 17540 moves, longest 232, drops 363, skipped 549, with a loser 132, without 68, 0 defects, 3441 ms
fuzz: 200 games, 19277 moves, longest 264, drops 357, skipped 607, with a loser 133, without 67, 0 defects, 3700 ms
```

**7** `{"op": "fuzz", "games": "300", "seed": "11"}` — row 1 again: the same numbers: as expected

```text
fuzz: 300 games, 52915 moves, longest 411, drops 0, skipped 0, with a loser 300, without 0, 0 defects, 10762 ms
```

2 100 games in all (the seven rows, with row 6 asked twice), no defect in any.

**A finding.** Row 6 asks for the real chance (`seed=0`). On the first run of this task it
answered the very numbers the plan had printed, `17728 moves, longest 252, drops 384`: the
fuzz built its repeating chance for every seed, and 0 was one more seed. The plan's
expectation was right and the code was not. `OZG_Fuzz.Run` now hands `OZG_Random` itself
to the games when the seed is 0. After the change the seeded rows gave the same numbers as
before, and row 6, asked twice, the two different answers above.

`log_verdict(source="server")` after the rows: **pass**, no warning and no error.

`client_compile_check(wait_seconds=120)`: **client compiles**.
