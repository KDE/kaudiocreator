
public:
  static EncoderPrefs *prefs(const QString &groupName);
  static bool hasPrefs(const QString &groupName);
  static void deletePrefs(const QString &groupName);

private:
  static QHash<QString, EncoderPrefs *> *m_prefs;
