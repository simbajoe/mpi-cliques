# Каталоги
IDIR = include
CDIR = src
ODIR = bin
TDIR = test
# Используемые общесистемные утилиты
CC := mpicc
CCTV := mpicc-TV
# Флаги компиляции
CFLAGS := -I$(IDIR)
# Главная программа
PROG = cliques

# По-умолчанию и по make all собираем программу.
# Отдельно отчёт собирается как make report
.PHONY: all
all: $(PROG)

CSRC := $(wildcard $(CDIR)/*.c)

$(PROG):
	$(CCTV) $^ -o $@ $(CFLAGS) $(CSRC)

clean:
	rm -rf cliques cliques.stud021.*
