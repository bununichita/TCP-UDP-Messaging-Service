# Bunu Nichita-Adrian 323CA nichita_adrian.bunu@stud.acs.upb.ro
# Aplicatie client-server TCP si UDP pentru gestionarea mesajelor

"subscriber.cpp" simuleaza functionalitatea unui client ce
trimite si primeste pachete TCP

"server.cpp" simuleaza functionalitatea unui router ce primeste
pachete TCP de la clienti si pachete UDP de la topic-uri

# Subscriber.cpp

Implementarea porneste de la scheletul laboratorului 7.

Subscribe - Primeste pachete tcp de la router cu topicul la care
            a dat subscribe

Unsubscribe - Nu mai primeste pachete tcp de la router cu topicul
              la care a dat unsubscribe

Cand clientul primeste un pachet TCP care contine pachetul UDP
primit de router, acesta afiseaza informatia din el in functie
de tipul mesajului (INT, SHORT_REAL, FLOAT, STRING).

Programul tine cont de posibilele erori in timpul functionarii
(DIE).

# 