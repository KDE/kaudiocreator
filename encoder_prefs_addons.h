
public:
  static EncoderPrefs *prefs(const QString &groupName);
  static bool hasPrefs(const QString &groupName);
  static void deletePrefs(const QString &groupName);

private:
  static QDict<EncoderPrefs> *m_prefs;
