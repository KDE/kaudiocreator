kinkattaicondir = $(kde_datadir)/kaudiocreator/icons
kinkattaicon_ICON = AUTO

ICONS = check.png

EXTRA_DIST = $(ICONS)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kaudiocreator/pics
	$(INSTALL_DATA) $(ICONS) $(DESTDIR)$(kde_datadir)/kaudiocreator/pics

uninstall-local:
	cd $(DESTDIR)$(kde_datadir)/kaudiocreator/pics && rm -f $(ICONS)
	-rmdir $(DESTDIR)$(kde_datadir)/kaudiocreator/pics
