function showHide(){
  var checkboxRain=document.getElementById("rain");
  var checkboxSoil=document.getElementById("soil");
  var checkboxWater=document.getElementById("water");
  var hiddenRain = document.getElementsByClassName("rain");
  var hiddenSoil = document.getElementsByClassName("soil");
  var hiddenWater = document.getElementsByClassName("water");


  for(var i=0 ; i !=hiddenRain.length; i++){
    if (checkboxRain.checked){
      hiddenRain[i].style.display="inline";
    }
    else{
      hiddenRain[i].style.display="none";
    }
  }
for(var i=0 ; i !=hiddenSoil.length; i++){
  if (checkboxSoil.checked){
    hiddenSoil[i].style.display="inline";
  }
  else{
    hiddenSoil[i].style.display="none";
  }
}

for(var i=0 ; i !=hiddenRain.length; i++){
if (checkboxWater.checked){
  hiddenWater[i].style.display="inline";
}
else{
  hiddenWater[i].style.display="none";
}
}
}

function openNav() {
    document.getElementById("mySidenav").style.width = "170px";
}

/* Set the width of the side navigation to 0 */
function closeNav() {
  document.getElementById("mySidenav").style.width = "0";
}
