
# ash (acorn shell) Makefile

MFLAGS = --no-print-directory

.PHONY: ash

ash:
	-@$(MAKE) $(MFLAGS) -C ash all

clean:
	-@$(MAKE) $(MFLAGS) -C ash clean
