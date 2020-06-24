.code32
.file "zad5s.s"

.data
maska1: .quad 0b1111100011111000000000000000000000000000000000000000000000000000
maska2: .quad 0b0000000000000000000000000000000011111000000000000000000000000000
sos: .quad 0b0001111100011111000000000000000000011111000000000000000000000000
shift: .quad 3
zeros: .quad 0
.globl filter
.type filter, @function
filter:
pushl %ebp #zabezpieczam stary %ebp
movl %esp, %ebp #kopiuje do %ebp aktualny wskaznik stosu
pushl %ebx
pushl %eax #Poniewaz funkcja nie zwraca wartosci, na koniec przywroce pocztkowy %eax
movl 16(%ebp), %ecx #Zostanie zwiekszone o 1 i konwersja sie zacznie na nie-krawedzi
movl 20(%ebp), %eax
subl $1, %eax
mull 16(%ebp)
subl $2, %eax #skonczy sie w prawym dolnym rogu "nie-krawedziowych bajtow"
movl %eax, %edx #Obliczanie ilosci iteracji petli

petla:
incl %ecx

#Przejscie do kolejnej iteracji, jesli bajt jest na krawedzi obrazka
#Mialo sie zignorowac krawedzie, a to sprawdzenie wymaga wielu instrukcji, wiec
#zakomentowalem to sprawdzanie
#pushl %edx
#movl %ecx, %eax
#movl $0, %edx
#divl 16(%ebp)
#cmpl %edx, %ecx
#jz petla
#incl %ecx
#movl %ecx, %eax
#movl $0, %edx
#divl 16(%ebp)
#decl %ecx
#cmpl %edx, %ecx
#jz petla
#popl %edx

#Ustawianie %mm0 i %mm1
subl 16(%ebp), %ecx #*
subl $1, %ecx
movl 8(%ebp), %eax
movq (%eax,%ecx,1), %mm1
pand maska1, %mm1
addl 16(%ebp), %ecx #**
movd (%eax,%ecx,1), %mm2
pand maska2, %mm2
paddb %mm2, %mm1
addl $2, %ecx #****
movd (%eax,%ecx,1), %mm2
pand maska2, %mm2
subl $1, %ecx #***
addl 16(%ebp), %ecx
movq 8(%eax,%ecx,1), %mm0
pand maska1, %mm0
paddb %mm2, %mm0
subl 16(%ebp), %ecx

#tu w %mm0 sa bajty: (i+width).(i+width+1).0.0.(i+1).0.0.0
#tu w %mm1 sa bajty: (i-width-1).(i-width).0.0.(i-1).0.0.0

psrlw shift, %mm1        #Podzielenie przez 8 !bajtow! w %mm1 - WAZNE -> 1 w maskach 
psrlw shift, %mm0        #Podzielenie przez 8 !bajtow! w %mm0
paddb sos, %mm0          #Dodanie do kazdego bajtu w %mm0 cechy ilorazu 255/8
psubw %mm1, %mm0         #Odjecie slow: %mm0=%mm0-%mm1
psadbw zeros, %mm0       #Slowa w %mm0: 0.0.0.wynik
movd %mm0, %ebx          #Skopiowanie wyniku z %mm0 do %ebx
movl 12(%ebp), %eax      #Skopiowanie adresu poczatkowego bajta obrazka wyjsciowego
movb %bl, (%eax,%ecx,1)  #Podstawienie pod bajt i-ty obrazka wynikowego

cmpl %edx, %ecx
jl petla
popl %eax
popl %ebx
emms
movl %ebp, %esp #odtwarzam stary stos
popl %ebp #odtwarzam stary %ebp
ret

.globl func
.type func, @function
func:
pushl %ebp
movl %esp, %ebp
subl $4, %esp
pushl %ebx  #zapamietujemy %ebx, zeby zachowac zgodnosc z ABI
xorl %eax, %eax
cpuid
rdtsc
popl %ebx
movl %ebp, %esp
popl %ebp
ret

.globl func2
.type func2, @function
func2:
pushl %ebp
movl %esp, %ebp
movl %edx, %eax
movl %ebp, %esp
popl %ebp
ret

