.PHONY: all clean

FIXTURE := search_result_entry_large.bin

all: $(FIXTURE)

$(FIXTURE): gen_ldap_fixture.py
	python3 gen_ldap_fixture.py $(FIXTURE)

clean:
	rm -f $(FIXTURE)

cleanall: clean
