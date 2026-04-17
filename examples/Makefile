
.PHONY: all clean cleanall

all:
	@$(MAKE) all -C ldap
	@$(MAKE) all -C krb5
	@$(MAKE) all -C snmp
	@$(MAKE) all -C goose
	@$(MAKE) all -C sv
#	@$(MAKE) all -C mms

run_example:
	@$(MAKE) run_example -C ldap
	@$(MAKE) run_example -C krb5
	@$(MAKE) run_example -C snmp
	@$(MAKE) run_example -C goose
	@$(MAKE) run_example -C sv
	#@$(MAKE) run_example -C mms

clean:
	@$(MAKE) clean -C ldap
	@$(MAKE) clean -C krb5
	@$(MAKE) clean -C snmp
	@$(MAKE) clean -C goose
	@$(MAKE) clean -C sv
#	@$(MAKE) clean -C mms

cleanall:
	@$(MAKE) cleanall -C ldap
	@$(MAKE) cleanall -C krb5
	@$(MAKE) cleanall -C snmp
	@$(MAKE) cleanall -C goose
	@$(MAKE) cleanall -C sv
#	@$(MAKE) cleanall -C mms
