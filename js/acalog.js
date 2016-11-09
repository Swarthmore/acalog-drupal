(function ($) {
	$(document).ready(function(){
		$("a.acalog-course-link").click(function(event){
			event.preventDefault();
			if (screen.height) {
				var LEFT = (screen.width - 770) / 2;
				var TOP = (screen.height - 600) / 2;
			} else {
				var LEFT = (screen.availWidth - 770) / 2;
				var TOP = (screen.availHeight - 600) / 2;
			}
			acalogcoursewindow = window.open($(this).attr('href'), 'acalogcoursewindow', 'height=600,width=770,resizable=yes,scrollbars=yes,menubar=no,toolbar=no,personalbar=no,statusbar=no,locationbar=no,screenX='+LEFT+',screenY='+TOP+',left='+LEFT+',top='+TOP);
			acalogcoursewindow.focus();
		});
	});
})(jQuery);