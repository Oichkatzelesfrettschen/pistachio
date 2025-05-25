---- MODULE TicketLock ----
EXTENDS Naturals, TLC

CONSTANTS N

VARIABLES pc, ticket, next, owner

ProcSet == 0..N-1

vars == << pc, ticket, next, owner >>

Init == 
    /\ pc = [i \in ProcSet |-> "try"]
    /\ ticket = [i \in ProcSet |-> -1]
    /\ next = 0
    /\ owner = 0

Acquire(i) ==
    /\ i \in ProcSet
    /\ pc[i] = "try"
    /\ pc' = [pc EXCEPT ![i] = "wait"]
    /\ ticket' = [ticket EXCEPT ![i] = next]
    /\ next' = next + 1
    /\ UNCHANGED owner

Wait(i) ==
    /\ i \in ProcSet
    /\ pc[i] = "wait"
    /\ owner = ticket[i]
    /\ pc' = [pc EXCEPT ![i] = "cs"]
    /\ UNCHANGED <<ticket, next, owner>>

Release(i) ==
    /\ i \in ProcSet
    /\ pc[i] = "cs"
    /\ pc' = [pc EXCEPT ![i] = "done"]
    /\ owner' = owner + 1
    /\ UNCHANGED <<ticket, next>>

Next ==
    \E i \in ProcSet: Acquire(i) \/ Wait(i) \/ Release(i)

Spec == Init /\ [][Next]_vars

MutualExclusion ==
    \A i,j \in ProcSet: (i # j) => ~(pc[i] = "cs" /\ pc[j] = "cs")

FairnessInv ==
    \A i,j \in ProcSet:
        (ticket[i] < ticket[j] /\ pc[j] = "cs") => pc[i] = "done"

====
