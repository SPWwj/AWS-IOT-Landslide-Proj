var limit = 60 * 1,
    duration = 1000,
    now = new Date(Date.now() - duration)

var width = window.innerWidth,
    height = 300

var groups = {
    rain: {
        value: 0,
        color: 'orange',
        data: d3.range(limit).map(function() {
            return 0
        }),
        class: "rain"
    },
    soil: {
        value: 0,
        color: 'green',
        data: d3.range(limit).map(function() {
            return 0
        }),
        class: "soil"
    },
    water: {
        value: 0,
        color: 'grey',
        data: d3.range(limit).map(function() {
            return 0
        }),
    }
}

var x = d3.time.scale()
    .domain([now - (limit - 2), now - duration])
    .range([0, width])

var y = d3.scale.linear()
    .domain([0, 100])
    .range([height, 10])

var line = d3.svg.line()
    .interpolate('basis')
    .x(function(d, i) {
        return x(now - (limit - 1 - i) * duration)
    })
    .y(function(d) {
        return y(d)
    })

var svg = d3.select('.graph').append('svg')
    .attr('class', 'chart')
    .attr('width', width)
    .attr('height', height + 50)

var axis = svg.append('g')
    .attr('class', 'x axis')
    .attr('transform', 'translate(50,' + (height)+ ')')
    .call(x.axis = d3.svg.axis().scale(x).orient('bottom'))

var axisY = d3.svg.axis()
    .scale(y)
    .orient("left")
    .ticks(10)
    .tickPadding(0)

var YGuide =d3.select('.chart')
    .append('g')
    axisY(YGuide)
    YGuide.attr('transform', 'translate(50,0)')
    YGuide.selectAll('path')
    .style('fill','none')
    .style('stroke','#000')
    YGuide.selectAll('line')
    .style('stroke','#000')

var paths = svg.append('g')

for (var name in groups) {
    var group = groups[name]
    group.path = paths.append('path')
        .data([group.data])
        .attr('class', name + ' group')
        .style('stroke', group.color)
}

function tick(loadedData) {
now = new Date()

    // Add new values
    groups.water.value=parseFloat((loadedData["ID0"][6]*100).toFixed(1))
    groups.soil.value=parseFloat((loadedData["ID0"][5]*100).toFixed(1))
    groups.rain.value=parseFloat((loadedData["ID0"][4]*100).toFixed(1))
    for (var name in groups) {
        var group = groups[name]
        group.data.push(group.value) // Real values arrive at irregular intervals
        // group.data.push(loadedData["ID0"][3]*100)
        group.path.attr('d', line)
    }

    // Shift domain
    x.domain([now - (limit - 2) * duration, now - duration])

    // Slide x-axis left
    axis.transition()
        .duration(duration)
        .ease('linear')
        .call(x.axis)

    // Slide paths left
    paths.attr('transform', null)
        .transition()
        .duration(duration)
        .ease('linear')
        .attr('transform', 'translate(' + x(now - (limit - 1) * duration) + ')')
        // .each('end', tick)

    // Remove oldest data point from each group
    for (var name in groups) {
        var group = groups[name]
        group.data.shift()
    }
}
