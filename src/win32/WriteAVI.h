#include <vfw.h>

class CAVIFile {
public:
  CAVIFile();
  virtual ~CAVIFile();

  bool Open(const char *filename);
  virtual bool AddFrame(const char * bmp);
  virtual bool IsOK() const            {return bOK;};
  void SetRate(int r) { rate = r; }
  void SetFormat(BITMAPINFOHEADER *);
  bool IsSoundAdded() { return soundAdded; }
  void SetSoundFormat(WAVEFORMATEX *);
  bool AddSound(const char *sound, int len);

private:
  int rate;

  WAVEFORMATEX soundFormat;
  BITMAPINFOHEADER bitmap;
  AVISTREAMINFO strhdr;
  AVISTREAMINFO soundhdr;
  PAVIFILE pfile;
  PAVISTREAM ps;
  PAVISTREAM psCompressed;
  PAVISTREAM psound;
  AVICOMPRESSOPTIONS opts;
  AVICOMPRESSOPTIONS FAR * aopts[1];
  int nFrames;
  bool bOK;
  bool soundAdded;
};

