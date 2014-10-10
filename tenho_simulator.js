var TenhoSimulator = (function() {
  var NACL_MODULE = null;

  var TYPE_MAX   = 4; // 一般, 上級, 特上, 鳳凰
  var LENGTH_MAX = 2; // 東風, 東南
  var GRADE_MAX  = 21; // 段位
  
  // 必要ポイント
  var PROMOTED_POINT = [
    30,   30,   30,   60,   60,   60,   90,  100,  100,  100,
    400, 800, 1200, 1600, 2000, 2400, 2800, 3200, 3600, 4000
  ];
  
  // 1位獲得ポイント
  var ACQUIRED_POINT_1ST = [
    [ 30, 45 ], // 一般
    [ 40, 60 ], // 上級
    [ 50, 20 ], // 特上
    [ 60, 90 ]  // 鳳凰
  ];
  
  // 2位獲得ポイント
  var ACQUIRED_POINT_2ND = [
    [  0,  0 ], // 一般
    [ 10, 15 ], // 上級
    [ 20, 30 ], // 特上
    [ 30, 45 ]  // 鳳凰
  ];
  
  // 4位獲得点数
  var ACQUIRED_POINT_4TH = [
    [    0,    0 ], // 0: 新人
    [    0,    0 ], // 1: 9級
    [    0,    0 ], // 2: 8級
    [    0,    0 ], // 3: 7級
    [    0,    0 ], // 4: 6級
    [    0,    0 ], // 5: 5級
    [    0,    0 ], // 6: 4級
    [    0,    0 ], // 7: 3級
    [  -10,  -15 ], // 8: 2級
    [  -20,  -30 ], // 9: 1級
    [  -30,  -45 ], // 10: 初段
    [  -40,  -60 ], // 11: 二段
    [  -50,  -75 ], // 12: 三段
    [  -60,  -90 ], // 13: 四段
    [  -70, -105 ], // 14: 五段
    [  -80, -120 ], // 15: 六段
    [  -90, -135 ], // 16: 七段
    [ -100, -150 ], // 17: 八段
    [ -110, -165 ], // 18: 九段
    [ -120, -180 ], // 19: 十段
    [    0,    0 ]  // 20: 鳳凰位
  ];
  
  // 後段
  var SUBSEQUENT_STAGE = [
    false, false, false, false, false, false, false, false, false, false, // 新人 - 1級
    true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  // 初段 - 十段
    false                                                                 // 鳳凰位
  ];
  
  // プレイ可能条件
  var PERMISSION = [
    [  0, 13 ], // 一般: 0 (新人) <= x <= 12 (三段)
    [  9, 21 ], // 上級
    [ 14, 21 ], // 特上
    [ 16, 21 ]  // 鳳凰
  ];
  
  var GRADE_LIST = [
    "新人", "9級", "8級", "7級", "6級", "5級", "4級", "3級", "2級", "1級",
    "初段", "二段", "三段", "四段", "五段", "六段", "七段", "八段", "九段", "十段",
    "鳳凰位"
  ];
  
  // グラフテンプレート
  var GRAPH_TEMPLATE = {
    datasets: [
      {
        fillColor: "rgba(151,187,205,0.5)",
        strokeColor: "rgba(151,187,205,0.8)",
        highlightFill: "rgba(151,187,205,0.75)",
        highlightStroke: "rgba(151,187,205,1)"
      }
    ]
  };
  
  function postMessage(msg) {
    NACL_MODULE.postMessage(msg);
  }
  
  function initPnaclModule() {
    
    // 必要ポイント
    for (var i = 0; i < PROMOTED_POINT.length; ++i) {
      postMessage({
        setting: {
          key  : "promotedPoint",
          value: {
            grade: i,
            point: PROMOTED_POINT[i]
          }
        }
      });
    }
    
    // 獲得ポイント
    for (var i = 0; i < ACQUIRED_POINT_1ST.length; ++i) {
      for (var j = 0; j < LENGTH_MAX; ++j) {
        for (var k = 0; k < GRADE_MAX - 1; ++k) {
          postMessage({
            setting: {
              key  : "acquiredPoint",
              value: {
                type  : i,
                grade : k,
                rank  : 0,
                length: j,
                point : ACQUIRED_POINT_1ST[i][j]
              }
            }
          });
        }
      }
    }
    
    for (var i = 0; i < ACQUIRED_POINT_2ND.length; ++i) {
      for (var j = 0; j < LENGTH_MAX; ++j) {
        for (var k = 0; k < GRADE_MAX - 1; ++k) {
          postMessage({
            setting: {
              key  : "acquiredPoint",
              value: {
                type  : i,
                grade : k,
                rank  : 1,
                length: j,
                point : ACQUIRED_POINT_2ND[i][j]
              }
            }
          });
        }
      }
    }
    
    for (var i = 0; i < ACQUIRED_POINT_4TH.length; ++i) {
      for (var j = 0; j < LENGTH_MAX; ++j) {
        for (var k = 0; k < TYPE_MAX; ++k) {
          postMessage({
            setting: {
              key  : "acquiredPoint",
              value: {
                type  : k,
                grade : i,
                rank  : 3,
                length: j,
                point : ACQUIRED_POINT_4TH[i][j]
              }
            }
          });
        }
      }
    }
    
    // 後段
    for (var i = 0; i < SUBSEQUENT_STAGE.length; ++i) {
      postMessage({
        setting: {
          key  : "isSubsequentStage",
          value: {
            grade            : i,
            isSubsequentStage: SUBSEQUENT_STAGE[i]
          }
        }
      });
    }
    
    // プレイ制限
    for (var i = 0; i < PERMISSION.length; ++i) {
      for (var j = 0; j < GRADE_MAX; ++j) {
        postMessage({
          setting: {
            key  : "permission",
            value: {
              type   : i,
              grade  : j,
              canPlay: PERMISSION[i][0] <= j && j <= PERMISSION[i][1]
            }
          }
        });
      }
    }
  }
  
  function startPnaclModule() {
    postMessage({ profile: { key: "point", value: getInt('#point') }});
    postMessage({ profile: { key: "grade", value: getInt('#grade') }});
    
    postMessage({ profile: { key: "rate_1st", value: getDouble('#rate_1st') }});
    postMessage({ profile: { key: "rate_2nd", value: getDouble('#rate_2nd') }});
    postMessage({ profile: { key: "rate_3rd", value: getDouble('#rate_3rd') }});
    postMessage({ profile: { key: "rate_4th", value: getDouble('#rate_4th') }});
    
    postMessage({ compute: {
      loopCount: getInt('#loop_count'),
      setCount : getInt('#set_count'),
      length   : getInt('#length')
    }});
  }
  
  function drawGraph(data) {
    var template    = _.template($('#graph_template').text());
    var canvas_wrap = $('#graph_area');
    
    canvas_wrap.html('');
    canvas_wrap.html(template());
    
    var context = canvas_wrap.find('canvas')[0].getContext('2d');
    var graph   = _.clone(GRAPH_TEMPLATE);
    
    graph.datasets[0].data = data;
    graph.labels = _.clone(GRADE_LIST); // 何故かラベルが減っていくバグがあるため追加
    
    new Chart(context).Bar(graph).removeData();
  }
  
  function getDouble(selector) {
    var text = $(selector).val();
    
    if (text.charAt(0) == '.') {
      text = '0' + text;
    }
    
    if (text.indexOf('.') < 0) {
      return parseFloat(text) / 1000;
    }
    
    else {
      return parseFloat(text);
    }
  }
  
  function getInt(selector) {
    var text = $(selector).val();
    return parseInt(text, 10);
  }
  
  $(function() {
    $('.input').submit(function(event) {
      event.preventDefault();
      event.stopPropagation();
      
      try {
        startPnaclModule();
      }
      
      catch(e) {
        $('#graph_area').text('エラーが発生しました。未対応のブラウザのようです。');
      }
    });
  });
  
  return {
    onLoad: function () {
      $('#graph_area').text('ロード完了');
      NACL_MODULE = document.getElementById('pnacl_module');
      initPnaclModule();
    },
    
    onMessage: function(msg) {
      var data = msg.data;
      
      if (data.computed) {
        drawGraph(data.computed);
      }
    }
  };
})();
