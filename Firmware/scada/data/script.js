const iniciarBtn = document.getElementById('iniciarBtn');
const frenarBtn = document.getElementById('frenarBtn');
const descargarBtn = document.getElementById('descargarBtn');
let tiempo = 0;
let valores = [0, 0, 0];
let myChart;
let count = 0;
let intervalId;
let tiempoNominal = 0.0;
let datosAlmacenados = [];

function iniciarGrafica() {
    let campo_rpm = document.getElementById('rpm_field');
    let campo_voltaje = document.getElementById('voltaje_field');
    let campo_corriente = document.getElementById('corriente_field');
    let campo_tiempo_adquisicion = document.getElementById('tiempo_adquisicion_field');

    if (campo_rpm.value != '' && campo_voltaje.value != '' && campo_corriente.value != '' && campo_tiempo_adquisicion.value != '') {
        var xhttp = new XMLHttpRequest(); 
        xhttp.onreadystatechange = function () { 
            if (this.readyState == 4 && this.status == 200) { 
                console.log('Datos enviados: ' + campo_rpm.value + ' y ' + campo_voltaje.value + ' y ' + campo_corriente.value + ' y ' + campo_tiempo_adquisicion.value); 
            } 
        }; 
        xhttp.open('GET', '/enviar?campo1=' + campo_rpm.value + '&campo2=' + campo_voltaje.value + '&campo3=' + campo_corriente.value + '&campo4=' + campo_tiempo_adquisicion.value, true); 
        xhttp.send();
        iniciarBtn.hidden = true;
        frenarBtn.style.display = 'block';
        descargarBtn.disabled = true;
        descargarBtn.classList.add('disabled-btn');
        valores = [0, 0, 0];
        tiempo = 0;
        count = 0;
        if (myChart) {
            myChart.destroy();
        }
        const ctx = document.getElementById('myChart').getContext('2d');
        myChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [tiempo.toString()],
                datasets: [{
                    label: 'RPM',
                    data: [valores[0]],
                    borderColor: 'rgba(75, 192, 192, 1)',
                    borderWidth: 2,
                    fill: false
                }, {
                    label: 'Voltaje',
                    data: [valores[1]],
                    borderColor: 'rgba(255, 99, 132, 1)',
                    borderWidth: 2,
                    fill: false
                }, {
                    label: 'Corriente',
                    data: [valores[2]],
                    borderColor: 'rgba(255, 206, 86, 1)',
                    borderWidth: 2,
                    fill: false
                }]
            },
            options: {
                scales: {
                    x: {
                        type: 'linear',
                        position: 'bottom',
                        min: 'auto',
                        max: 'auto'
                    },
                    y: {
                        min: 'auto',
                        max: 'auto'
                    }
                },
                animation: {
                    duration: 0
                }
            }
        });

    }
}

function plotData(jsonValue){
    var keys = Object.keys(jsonValue);
    let tiempoAct = parseFloat(jsonValue[keys[0]]);
    let rpm = parseFloat(jsonValue[keys[1]]);
    let voltaje = parseFloat(jsonValue[keys[2]]);
    let corriente = parseFloat(jsonValue[keys[3]]);
    myChart.data.labels.push(tiempoAct);
    myChart.data.datasets[0].data.push(rpm);
    myChart.data.datasets[1].data.push(voltaje);
    myChart.data.datasets[2].data.push(corriente);
    myChart.update();
    if(myChart.data.labels.length  > 40){
        myChart.data.labels.shift();
        myChart.data.datasets[0].data.shift();
        myChart.data.datasets[1].data.shift();
        myChart.data.datasets[2].data.shift();
    }
    if (tiempoAct >= parseFloat(document.getElementById('tiempo_adquisicion_field').value)) {
        detenerCaptura();
    }
}

function detenerCaptura(){
    frenarBtn.style.display = 'none';
    iniciarBtn.hidden = false;
    descargarBtn.disabled = false;
    descargarBtn.classList.remove('disabled-btn');
    fetch('/detenerGrafica', { method: 'GET' });
}

function limpiarGrafica() {
    myChart.data.labels = [];
    myChart.data.datasets.forEach(dataset => {
        dataset.data = [];
    });
    myChart.update();
}

function descargarDatos() {
    let contenido = '';
    datosAlmacenados.forEach((dato) => {
        contenido += `${dato.tiempo}\t${dato.rpm}\t${dato.voltaje}\t${dato.corriente}\n`; // Cambiar esto por la forma de archivos que se comentó
    });

    const blob = new Blob([contenido], { type: 'text/plain' });

    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    const fechaActual = new Date();
    const formatoFechaHora = `${fechaActual.toLocaleDateString()} ${fechaActual.toLocaleTimeString()}`;
    a.download = formatoFechaHora + '.txt';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
}

// Funcion para obtener las ultimas lecturas en la pagina web cuando esta carga por primera vez
// Esta funcion se puede utilizar para tener la grafica activa. ESTA NO ES LA FUNCION PARA GRAFICAR DATOS EN EL APARTADO DE EXPERIMENTACION
function getReadings() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var myObj = JSON.parse(this.responseText);
            console.log(myObj);
            plotData(myObj);
        }
    };
    xhr.open("GET", "/readings", true);
    xhr.send();
}

if (!!window.EventSource) {
    var source = new EventSource('/events');

    source.addEventListener('open', function (e) {
        console.log("Events Connected");
    }, false);

    source.addEventListener('error', function (e) {
        if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
        }
    }, false);

    source.addEventListener('message', function (e) {
        console.log("message", e.data);
    }, false);

    source.addEventListener('new_readings', function (e) {
        console.log("new_readings", e.data);
        var myObj = JSON.parse(e.data);
        console.log(myObj);
        plotData(myObj);
    }, false);
}

document.getElementById('iniciarBtn').addEventListener('click', iniciarGrafica);
document.getElementById('frenarBtn').addEventListener('click', detenerCaptura);
document.getElementById('descargarBtn').addEventListener('click', descargarDatos);