bin_SCRIPTS = disnix-instantiate disnix-manifest disnix-gendist-roundrobin disnix-copy-closure disnix-copy-snapshots disnix-delegate disnix-deploy disnix-env disnix-migrate disnix-ssh-client
pkgdata_SCRIPTS = checks
noinst_DATA = disnix-instantiate.1.xml disnix-manifest.1.xml disnix-gendist-roundrobin.1.xml disnix-copy-closure.1.xml disnix-copy-snapshots.1.xml disnix-delegate.1.xml disnix-deploy.1.xml disnix-env.1.xml disnix-migrate.1.xml disnix-ssh-client.1.xml

disnix-instantiate.1: disnix-instantiate.in
	$(HELP2MAN) --output=$@ --no-info --name 'Instantiate a distributed derivation from Disnix expressions' "$(SHELL) disnix-instantiate"

disnix-instantiate.1.xml: disnix-instantiate.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-manifest.1: disnix-manifest.in
	$(HELP2MAN) --output=$@ --no-info --name 'Generate a manifest file from Disnix expressions' "$(SHELL) disnix-manifest"

disnix-manifest.1.xml: disnix-manifest.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-migrate.1: disnix-migrate.in
	$(HELP2MAN) --output=$@ --no-info --name 'Migrates state from services that have been moved from one machine to another' "$(SHELL) disnix-migrate"

disnix-migrate.1.xml: disnix-migrate.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-gendist-roundrobin.1: disnix-gendist-roundrobin.in
	$(HELP2MAN) --output=$@ --no-info --name 'Generate a distribution expression from a service and infrastructure expression' "$(SHELL) disnix-gendist-roundrobin"

disnix-gendist-roundrobin.1.xml: disnix-gendist-roundrobin.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-copy-closure.1: disnix-copy-closure.in
	$(HELP2MAN) --output=$@ --no-info --name 'Copy a closure from or to a remote machine through a Disnix interface' "$(SHELL) disnix-copy-closure"

disnix-copy-closure.1.xml: disnix-copy-closure.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-copy-snapshots.1: disnix-copy-snapshots.in
	$(HELP2MAN) --output=$@ --no-info --name 'Copy a set of snapshots from or to a remote machine through a Disnix interface' "$(SHELL) disnix-copy-snapshots"

disnix-copy-snapshots.1.xml: disnix-copy-snapshots.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-delegate.1: disnix-delegate.in
	$(HELP2MAN) --output=$@ --no-info --name 'Delegates service builds to the target machines' "$(SHELL) disnix-delegate"

disnix-delegate.1.xml: disnix-delegate.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-deploy.1: disnix-deploy.in
	$(HELP2MAN) --output=$@ --no-info --name 'Deploys a prebuilt Disnix manifest' "$(SHELL) disnix-deploy"

disnix-deploy.1.xml: disnix-deploy.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-env.1: disnix-env.in
	$(HELP2MAN) --output=$@ --no-info --name 'Installs or updates the environment of a distributed system' "$(SHELL) disnix-env"

disnix-env.1.xml: disnix-env.1
	$(SHELL) ../maintenance/man2docbook.bash $<

disnix-ssh-client.1: disnix-ssh-client.in
	$(HELP2MAN) --output=$@ --no-info --name 'Provides access to the disnix-service through a SSH interface' "$(SHELL) disnix-ssh-client"

disnix-ssh-client.1.xml: disnix-ssh-client.1
	$(SHELL) ../maintenance/man2docbook.bash $<

man1_MANS = disnix-instantiate.1 disnix-manifest.1 disnix-migrate.1 disnix-gendist-roundrobin.1 disnix-copy-closure.1 disnix-copy-snapshots.1 disnix-delegate.1 disnix-deploy.1 disnix-env.1 disnix-ssh-client.1

EXTRA_DIST = checks disnix-instantiate.in disnix-manifest.in disnix-gendist-roundrobin.in disnix-copy-closure.in disnix-copy-snapshots.in disnix-delegate.in disnix-deploy.in disnix-env.in disnix-ssh-client.in \
  disnix-instantiate.1 disnix-manifest.1 disnix-migrate.1 disnix-gendist-roundrobin.1 disnix-copy-closure.1 disnix-copy-snapshots.1 disnix-delegate.1 disnix-deploy.1 disnix-env.1 disnix-ssh-client.1 \
  disnix-instantiate.1.xml disnix-manifest.1.xml disnix-migrate.1.xml disnix-gendist-roundrobin.1.xml disnix-copy-closure.1.xml disnix-copy-snapshots.1.xml disnix-delegate.1.xml disnix-deploy.1.xml disnix-env.1.xml disnix-ssh-client.1.xml
