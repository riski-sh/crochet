function switchtheme()
{

  let root = document.styleSheets[0].rules[0];

  let base00 = root.style.getPropertyValue('--base00');
  let base01 = root.style.getPropertyValue('--base01');
  let base02 = root.style.getPropertyValue('--base02');
  let base03 = root.style.getPropertyValue('--base03');

  let base0 = root.style.getPropertyValue('--base0');
  let base1 = root.style.getPropertyValue('--base1');
  let base2 = root.style.getPropertyValue('--base2');
  let base3 = root.style.getPropertyValue('--base3');

  root.style.setProperty('--base0', base00);
  root.style.setProperty('--base1', base01);
  root.style.setProperty('--base2', base02);
  root.style.setProperty('--base3', base03);

  root.style.setProperty('--base00', base0);
  root.style.setProperty('--base01', base1);
  root.style.setProperty('--base02', base2);
  root.style.setProperty('--base03', base3);
}
