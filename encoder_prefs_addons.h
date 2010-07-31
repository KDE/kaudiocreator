
public:
  static EncoderPrefs *prefs(const TQString &groupName);
  static bool hasPrefs(const TQString &groupName);
  static void deletePrefs(const TQString &groupName);

private:
  static TQDict<EncoderPrefs> *m_prefs;
