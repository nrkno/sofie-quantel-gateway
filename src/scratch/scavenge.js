const { Quantel } = require('../..');

/* async function run() {
  await Quantel.getISAReference('http://xproqisa01');
  let allCs = await Quantel.searchClips({ Title: '*', limit: 100000, idOnly: 1 });
  console.log(allCs.length);

  let types = {};
  //console.log(allCs.slice(1000, 2000))
  for ( let x = 0 ; x < allCs.length ; x += 2) {
    let frags = await Promise.all([
      Quantel.getFragments({ clipID: allCs[x] }),
      Quantel.getFragments({ clipID: allCs[x+1] })])
    frags[0].fragments.forEach(f => {
      types[f.type] = types[f.type] ? types[f.type] + 1 : 1;
    });
    frags[1].fragments.forEach(f => {
      types[f.type] = types[f.type] ? types[f.type] + 1 : 1;
    });
  }
  console.log(types);
} */

async function run() {
  await Quantel.getISAReference('http://xproqisa02');
  let allCs = await Quantel.searchClips({ Title: '*', limit: 100000, idOnly: 1 });
  console.log(allCs.length);

  for ( let x = 8650 ; x >= 0; x-- ) {
    let clip = await Quantel.getClipData({ clipID: allCs[x] });
    if (isNaN(+clip.Frames) || +clip.Frames < 1) {
      console.log(x, ':', allCs[x], '=', clip.Frames);
    }
  }
}

run();
